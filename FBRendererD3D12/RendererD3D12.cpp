#include "pch.h"
#include "RendererD3D12.h"
#include "Util.h"
#include "CommandAllocator.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "UploadBuffer.h"
#include "Shader.h"
#include "ConverterD3D12.h"
#include "RootSignature.h"
#include <iostream>
#include "../../FBCommon/StringHeader.h"

using namespace fb;
using Microsoft::WRL::ComPtr;
RendererD3D12* fb::gRendererD3D12 = nullptr;

extern "C" {
	fb::IRenderer* CreateRendererD3D12()
	{
		return new RendererD3D12();
	}
}

static void Test()
{

}

bool RendererD3D12::Initialize(void* windowHandle)
{
	std::cout << "Initializing Renderer D3D12." << std::endl;
	gRendererD3D12 = this;

	Test();

	WindowHandle = (HWND)windowHandle;
#ifdef _DEBUG
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&DXGIFactory)));

	ThrowIfFailed(D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&Device)));

	ThrowIfFailed(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&Fence)));

	FenceEventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

	RtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DsvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CbvSrvUavDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = BackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(Device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	Msaa4xQuality = msQualityLevels.NumQualityLevels;
	//Msaa4xState = true;
	assert(Msaa4xQuality > 0 && "Unexpected MSAA quality level.");

#ifdef _DEBUG
	//LogAdapters();
#endif

	CreateCommandObjects();
	CreateSwapChain();
	CreateRtvAndDsvDescriptorHeaps();

	OnResized();

	std::cout << "Finish Initialization." << std::endl;
	return true;
}

void RendererD3D12::Finalize()
{
	PSOs.clear();
	CloseHandle(FenceEventHandle);
	gRendererD3D12 = nullptr;
	delete this;
}

void RendererD3D12::WaitFence(UINT64 fence)
{
	if (fence == 0)
		return;
	if (Fence->GetCompletedValue() < fence) {
		ThrowIfFailed(Fence->SetEventOnCompletion(fence, FenceEventHandle));
		WaitForSingleObject(FenceEventHandle, INFINITE);
	}
}

int RendererD3D12::GetNumSwapchainBuffers()
{
	return SwapChainBufferCount;
}

void RendererD3D12::PrepareDescriptorHeap(EDescriptorHeapType heapType, UINT count)
{
	switch (heapType)
	{
	case EDescriptorHeapType::Default:
	{
		
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
		heapDesc.NumDescriptors = count;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.NodeMask = 0;
		ThrowIfFailed(Device->CreateDescriptorHeap(&heapDesc,
			IID_PPV_ARGS(&DefaultDescriptorHeap)));
		break;
	}
	}
}

void RendererD3D12::OnResized()
{
	if (!Device || !SwapChain || !DirectCmdAllocator)
		return;

	FlushCommandQueue();
	ThrowIfFailed(CommandList->Reset(DirectCmdAllocator.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < SwapChainBufferCount; ++i)
		SwapChainBuffer[i].Reset();

	DepthStencilBuffer.Reset();

	auto clientWidth = GetClientWidth();
	auto clientHeight = GetClientHeight();
	// Resize the swap chain.
	ThrowIfFailed(SwapChain->ResizeBuffers(
		SwapChainBufferCount,
		clientWidth, clientHeight,
		BackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	CurrBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffer[i])));
		Device->CreateRenderTargetView(SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, RtvDescriptorSize);
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = clientWidth;
	depthStencilDesc.Height = clientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	depthStencilDesc.SampleDesc.Count = Msaa4xState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = Msaa4xState ? (Msaa4xQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(DepthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	Device->CreateDepthStencilView(DepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	// Transition the resource from its initial state to be used as a depth buffer.
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(DepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	// Execute the resize commands.
	ThrowIfFailed(CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	SignalFence();
	// Wait until resize is complete.
	FlushCommandQueue();

	// Update the viewport transform to cover the client area.
	ScreenViewport.TopLeftX = 0;
	ScreenViewport.TopLeftY = 0;
	ScreenViewport.Width = static_cast<float>(clientWidth);
	ScreenViewport.Height = static_cast<float>(clientHeight);
	ScreenViewport.MinDepth = 0.0f;
	ScreenViewport.MaxDepth = 1.0f;

	ScissorRect = { 0, 0, (LONG)clientWidth, (LONG)clientHeight };
}

ICommandAllocator* RendererD3D12::CreateCommandAllocator()
{
	auto ca = new CommandAllocator;
	ThrowIfFailed(Device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(ca->CommandAllocator.GetAddressOf())));
	return ca;
}

IVertexBuffer* RendererD3D12::CreateVertexBuffer(const void* vertexData, UINT size, UINT stride, bool keepData)
{
	auto vb = new VertexBuffer();
	if (!vb->Initialize(vertexData, size, stride, keepData))
	{
		delete vb; vb = nullptr;
	}
	return vb;
}

IIndexBuffer* RendererD3D12::CreateIndexBuffer(const void* indexData, UINT size, EDataFormat format, bool keepData)
{
	auto ib = new IndexBuffer();
	if (!ib->Initialize(indexData, size, format, keepData))
	{
		delete ib; ib = nullptr;
	}
	return ib;
}

IUploadBuffer* RendererD3D12::CreateUploadBuffer(UINT elementSize, UINT count, bool constantBuffer)
{
	auto ub = new UploadBuffer();
	if (!ub->Initialize(elementSize, constantBuffer ? 256 : 1, count))
	{
		delete ub;
		return nullptr;
	}

	return ub;
}

PSOID RendererD3D12::CreateGraphicsPipelineState(const FPSODesc& psoDesc)
{
	ComPtr<ID3D12PipelineState> pso;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = Convert(psoDesc);
	ThrowIfFailed(Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pso.GetAddressOf())));
	PSOs.insert(std::make_pair(NextPSOId, pso));
	return NextPSOId++;
}

void RendererD3D12::DestroyGraphicsPipelineState(PSOID psoid)
{
	PSOs.erase(psoid);
}

IShader* RendererD3D12::CompileShader(
	const char* filepath, FShaderMacro* macros, int numMacros, EShaderType shaderType, const char* entryFunctionName)
{
	auto shader = new Shader;
	D3D_SHADER_MACRO* d3dMacros = nullptr;
	if (numMacros > 0) {
		d3dMacros = new D3D_SHADER_MACRO[numMacros + 1];
		for (int i = 0; i < numMacros; ++i) {
			d3dMacros[i].Name = macros[i].Name;
			d3dMacros[i].Definition = macros[i].Def;
		}
		d3dMacros[numMacros].Name = nullptr;
		d3dMacros[numMacros].Definition = nullptr;
	}
	const char* shaderTarget = nullptr;
	switch (shaderType) {
	case EShaderType::PixelShader:
		shaderTarget = "ps_5_1";
		break;
	case EShaderType::VertexShader:
		shaderTarget = "vs_5_1";
		break;
	case EShaderType::GeometryShader:
		shaderTarget = "gs_5_1";
		break;
	case EShaderType::HullShader:
		shaderTarget = "hs_5_1";
		break;
	case EShaderType::DomainShader:
		shaderTarget = "ds_5_1";
		break;
	case EShaderType::ComputeShader:
		shaderTarget = "cs_5_1";
		break;
	}
	assert(shaderTarget != nullptr);
	try {
		shader->ByteCode = fb::CompileShader(
			AnsiToWString(filepath), d3dMacros, entryFunctionName, shaderTarget);
	}
	catch (const fb::DxException& ex) {
		ex.PrintErrorMessage();
		DebugBreak();
		delete shader; shader = nullptr;
	}
	delete[] macros;
	return shader;
}

EDataFormat RendererD3D12::GetBackBufferFormat() const
{
	return Convert(BackBufferFormat);
}

EDataFormat RendererD3D12::GetDepthStencilFormat() const
{
	return Convert(DepthStencilFormat);
}

IRootSignature* RendererD3D12::CreateRootSignature(const char* definition)
{	
	auto parameters = fb::Split(definition, ";");
	
	if (parameters.empty())
		return nullptr;

	CD3DX12_ROOT_PARAMETER* slotRootParameter = new CD3DX12_ROOT_PARAMETER[parameters.size()];
	std::vector<CD3DX12_DESCRIPTOR_RANGE*> descriptorRanges;
	int cpuIndex = 0;
	// type, num, gpu index
	for (const auto& param : parameters) 
	{
		auto paramItem = fb::Split(param, ",");
		if (paramItem[0] == std::string_view(RootDescriptorTableName))
		{
			assert(paramItem.size() == 3);
			int numDescriptors = atoi(std::string(paramItem[1]).c_str());
			int gpuIndex = atoi(std::string(paramItem[2]).c_str());
			descriptorRanges.push_back(new CD3DX12_DESCRIPTOR_RANGE);
			CD3DX12_DESCRIPTOR_RANGE* descriptorRange = descriptorRanges.back();
			descriptorRange->Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, numDescriptors, gpuIndex);
			slotRootParameter[cpuIndex].InitAsDescriptorTable(numDescriptors, descriptorRange);
		}
		else if (paramItem[0] == std::string_view(RootDescriptorCBVName))
		{
			int gpuIndex = atoi(std::string(paramItem[1]).c_str());
			int registerSpace = 0;
			if (paramItem.size() > 2)
				registerSpace = atoi(std::string(paramItem[1]).c_str());

			// TODO : shader visibility
			slotRootParameter[cpuIndex].InitAsConstantBufferView(gpuIndex, registerSpace);
		}
		else if (paramItem[0] == std::string_view(RootDescriptorSRVName))
		{
			int gpuIndex = atoi(std::string(paramItem[1]).c_str());
			int registerSpace = 0;
			if (paramItem.size() > 2)
				registerSpace = atoi(std::string(paramItem[1]).c_str());

			// TODO : shader visibility
			slotRootParameter[cpuIndex].InitAsShaderResourceView(gpuIndex, registerSpace);
		}
		else if (paramItem[0] == std::string_view(RootDescriptorUAVName))
		{
			int gpuIndex = atoi(std::string(paramItem[1]).c_str());
			int registerSpace = 0;
			if (paramItem.size() > 2)
				registerSpace = atoi(std::string(paramItem[1]).c_str());

			// TODO : shader visibility
			slotRootParameter[cpuIndex].InitAsUnorderedAccessView(gpuIndex, registerSpace);
		}
		else if (paramItem[0] == std::string_view(RootConstantName))
		{
			int gpuIndex = atoi(std::string(paramItem[1]).c_str());
			int numValues = atoi(std::string(paramItem[2]).c_str());
			int registerSpace = 0;
			slotRootParameter[cpuIndex].InitAsConstants(numValues, gpuIndex, registerSpace);
		}
		++cpuIndex;
	}

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc((UINT)parameters.size(), slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	RootSignature* rs = new RootSignature();


	ThrowIfFailed(Device->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(rs->RootSignature.GetAddressOf())));

	delete[] slotRootParameter;
	for (auto p : descriptorRanges) {
		delete p;
	}

	return rs;
}

int RendererD3D12::GetSampleCount() const
{
	return Msaa4xState ? 4 : 1;
}

int RendererD3D12::GetMsaaQuality() const
{
	assert(Msaa4xQuality - 1 >= 0);
	return Msaa4xState ? Msaa4xQuality - 1 : 0;
}

int RendererD3D12::GetBackbufferWidth() const
{
	return GetClientWidth();
}

int RendererD3D12::GetBackbufferHeight() const
{
	return GetClientHeight();
}

void RendererD3D12::ResetCommandList(ICommandAllocatorIPtr cmdAllocator, PSOID pso)
{
	bool set = false;
	if (pso != 0) {
		auto it = PSOs.find(pso);
		if (it != PSOs.end())
		{
			ThrowIfFailed(CommandList->Reset(
				cmdAllocator ? ((CommandAllocator*)cmdAllocator.get())->CommandAllocator.Get() : DirectCmdAllocator.Get(), 
				it->second.Get()));
			set = true;
		}
	}
	if (!set)
		ThrowIfFailed(CommandList->Reset(
			cmdAllocator ? ((CommandAllocator*)cmdAllocator.get())->CommandAllocator.Get() : DirectCmdAllocator.Get()
			, nullptr));
}

void RendererD3D12::CloseCommandList()
{
	ThrowIfFailed(CommandList->Close());
}

void RendererD3D12::ExecuteCommandList()
{
	ID3D12CommandList* cmdsLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}

void RendererD3D12::PresentAndSwapBuffer()
{
	ThrowIfFailed(SwapChain->Present(0, 0));
	CurrBackBuffer = (CurrBackBuffer + 1) % SwapChainBufferCount;
}

void RendererD3D12::TempResetCommandList()
{
	ThrowIfFailed(CommandList->Reset(DirectCmdAllocator.Get(), nullptr));
}

void RendererD3D12::BindDescriptorHeap(EDescriptorHeapType type)
{
	switch (type)
	{
	case EDescriptorHeapType::Default:
	{
		if (!DefaultDescriptorHeap)
			return;
		ID3D12DescriptorHeap* descriptorHeaps[] = { DefaultDescriptorHeap.Get() };
		CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		break;
	}
	}
}

void RendererD3D12::SetGraphicsRootConstantBufferView(int rootParamIndex, fb::IUploadBufferIPtr constantBuffer, int offset)
{
	auto gpuAddress = ((UploadBuffer*)constantBuffer.get())->Resource->GetGPUVirtualAddress();
	gpuAddress += offset;
	CommandList->SetGraphicsRootConstantBufferView(rootParamIndex, gpuAddress);
}

void RendererD3D12::SetGraphicsRootDescriptorTable(int rootParamIndex, fb::EDescriptorHeapType heapType, int index)
{
	switch (heapType) {
	case EDescriptorHeapType::Default:
	{
		auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(DefaultDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		passCbvHandle.Offset(index, CbvSrvUavDescriptorSize);
		CommandList->SetGraphicsRootDescriptorTable(rootParamIndex, passCbvHandle);
		break;
	}
	default:
		assert(0 && "Not implemented.");
	}
}

void RendererD3D12::SetGraphicsRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet, 
	const void* pSrcData, UINT DestOffsetIn32BitValues)
{
	CommandList->SetGraphicsRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet,
		pSrcData, DestOffsetIn32BitValues);
}

void RendererD3D12::TempCreateRootSignatureForSimpleBox()
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);
	
	ThrowIfFailed(Device->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(((RootSignature*)SimpleBoxRootSig.get())->RootSignature.GetAddressOf()))
	);
}

void RendererD3D12::TempBindVertexBuffer(const IVertexBufferIPtr& vb)
{
	auto d3dVB = (VertexBuffer*)vb.get();
	D3D12_VERTEX_BUFFER_VIEW views[] = { d3dVB->VertexBufferView() };
	CommandList->IASetVertexBuffers(0, 1, views);
}

void RendererD3D12::TempBindIndexBuffer(const IIndexBufferIPtr& ib)
{
	auto d3dIB = (IndexBuffer*)ib.get();
	auto view = d3dIB->IndexBufferView();
	CommandList->IASetIndexBuffer(&view);
}

void RendererD3D12::SetPrimitiveTopology(const fb::EPrimitiveTopology topology)
{
	CommandList->IASetPrimitiveTopology(Convert(topology));
}

void RendererD3D12::SetPipelineState(PSOID pso)
{
	auto it = PSOs.find(pso);
	if (it != PSOs.end())
	{
		CommandList->SetPipelineState(it->second.Get());
	}
}

void RendererD3D12::DrawIndexedInstanced(UINT IndexCountPerInstance,
	UINT InstanceCount,
	UINT StartIndexLocation,
	INT BaseVertexLocation,
	UINT StartInstanceLocation)
{
	CommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void RendererD3D12::DrawInstanced(UINT VertexCountPerInstance,
	UINT InstanceCount,
	UINT StartVertexLocation,
	UINT StartInstanceLocation)
{
	CommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void RendererD3D12::ResourceBarrier_Backbuffer_PresentToRenderTarget()
{
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
}

void RendererD3D12::ResourceBarrier_Backbuffer_RenderTargetToPresent()
{
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
}

void RendererD3D12::SetViewportAndScissor(int x, int y, UINT width, UINT height)
{
	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT rect = { 0, 0, (LONG)width, (LONG)height};

	CommandList->RSSetViewports(1, &viewport);
	CommandList->RSSetScissorRects(1, &rect);
}

void RendererD3D12::ClearRenderTargetDepthStencil()
{
	CommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
	CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void RendererD3D12::SetDefaultRenderTargets()
{
	CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
}

UINT64 RendererD3D12::SignalFence()
{
	auto fenceNumber = ++LastSignaledFenceNumber;
	CommandQueue->Signal(Fence.Get(), fenceNumber);
	return fenceNumber;
}

void RendererD3D12::TempBindRootDescriptorTable(UINT slot, EDescriptorHeapType type)
{
	switch (type) {
	case EDescriptorHeapType::Default:
	{
		CommandList->SetGraphicsRootDescriptorTable(slot, DefaultDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		break;
	}
	}

}

void RendererD3D12::TempDrawIndexedInstanced(UINT indexCount)
{
	CommandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}

void RendererD3D12::LogAdapters()
{
	UINT i = 0;
	IDXGIAdapter* adapter = nullptr;
	std::vector<IDXGIAdapter*> adapterList;
	while (DXGIFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wstring text = L"***Adapter: ";
		text += desc.Description;
		text += L"\n";

		OutputDebugString(text.c_str());

		IDXGIAdapter3* adapter3;
		adapter->QueryInterface(&adapter3);
		DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo;
		adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInfo);

		adapterList.push_back(adapter);

		++i;
	}

	for (size_t i = 0; i < adapterList.size(); ++i)
	{
		LogAdapterOutputs(adapterList[i]);
		ReleaseCom(adapterList[i]);
	}
}

void RendererD3D12::LogAdapterOutputs(IDXGIAdapter* adapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());

		LogOutputDisplayModes(output, BackBufferFormat);

		ReleaseCom(output);

		++i;
	}
}

void RendererD3D12::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;

	// Call with nullptr to get list count.
	output->GetDisplayModeList(format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flags, &count, &modeList[0]);

	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";

		::OutputDebugString(text.c_str());
	}
}

void RendererD3D12::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue)));

	ThrowIfFailed(Device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(DirectCmdAllocator.GetAddressOf())));

	ThrowIfFailed(Device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		DirectCmdAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(CommandList.GetAddressOf())));

	CommandList->Close();
}

void RendererD3D12::CreateSwapChain()
{
	// Release the previous swapchain we will be recreating.
	SwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = GetClientWidth();
	sd.BufferDesc.Height = GetClientHeight();
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = BackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = Msaa4xState ? 4 : 1;
	sd.SampleDesc.Quality = Msaa4xState ? (Msaa4xQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = WindowHandle;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	try {
		ThrowIfFailed(DXGIFactory->CreateSwapChain(
			CommandQueue.Get(),
			&sd,
			SwapChain.GetAddressOf()));
	}
	catch (const DxException& ex) {
		ex.PrintErrorMessage();
		DebugBreak();
	}
}

void RendererD3D12::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(Device->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(RtvHeap.GetAddressOf())));


	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(Device->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(DsvHeap.GetAddressOf())));
}

UINT RendererD3D12::GetClientWidth() const
{
	assert(WindowHandle);
	RECT r;
	GetClientRect(WindowHandle, &r);
	return r.right - r.left;
}

UINT RendererD3D12::GetClientHeight() const
{
	assert(WindowHandle);
	RECT r;
	GetClientRect(WindowHandle, &r);
	return r.bottom - r.top;
}

struct PendingUploaderRemovalInfo
{
	PendingUploaderRemovalInfo(ID3D12Resource* uploader)
		: Uploader(uploader) {}

	ID3D12Resource* Uploader;
};
std::vector<PendingUploaderRemovalInfo> PendingUploaderRemovalInfos;

void RendererD3D12::FlushCommandQueue()
{
	// Wait until the GPU has completed commands up to this fence point.
	if (LastSignaledFenceNumber != 0 && Fence->GetCompletedValue() < LastSignaledFenceNumber)
	{
		// Fire event when GPU hits current fence.  
		ThrowIfFailed(Fence->SetEventOnCompletion(LastSignaledFenceNumber, FenceEventHandle));
		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(FenceEventHandle, INFINITE);
	}

	for (auto info : PendingUploaderRemovalInfos)
	{
		if (info.Uploader)
			info.Uploader->Release();
	}
	PendingUploaderRemovalInfos.clear();
}

ID3D12Resource* RendererD3D12::CurrentBackBuffer()const
{
	return SwapChainBuffer[CurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE RendererD3D12::CurrentBackBufferView()const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		RtvHeap->GetCPUDescriptorHandleForHeapStart(),
		CurrBackBuffer,
		RtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE RendererD3D12::DepthStencilView()const
{
	return DsvHeap->GetCPUDescriptorHandleForHeapStart();
}

Microsoft::WRL::ComPtr<ID3D12Resource> RendererD3D12::CreateBufferInDefaultHeap(
	const void* initData,
	UINT64 byteSize)
{
	ComPtr<ID3D12Resource> defaultHeapBuffer;
	ID3D12Resource* uploadHeapBuffer = nullptr;

	ThrowIfFailed(Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultHeapBuffer.GetAddressOf())));

	ThrowIfFailed(Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadHeapBuffer)));

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultHeapBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources<1>(CommandList.Get(), defaultHeapBuffer.Get(), uploadHeapBuffer, 0, 0, 1, &subResourceData);
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultHeapBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	// going to delete the upload buffer after commands are flushed.
	PendingUploaderRemovalInfos.push_back(PendingUploaderRemovalInfo(uploadHeapBuffer));

	return defaultHeapBuffer;
}

void RendererD3D12::Bind(ID3D12RootSignature* rootSig)
{
	CommandList->SetGraphicsRootSignature(rootSig);
}

void RendererD3D12::BuildDescriptorHeaps()
{

}