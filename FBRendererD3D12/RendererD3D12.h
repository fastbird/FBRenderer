#pragma once
#include "../IRenderer.h"
namespace fb
{
	struct LiveObjectReporter
	{
		~LiveObjectReporter();
	};
	class RendererD3D12 : public IRenderer
	{
		// Should be placed on the top
		LiveObjectReporter _LiveObjectReporter;

		Microsoft::WRL::ComPtr<IDXGIFactory4> DXGIFactory;
		Microsoft::WRL::ComPtr<ID3D12Device> Device;
		Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
		HANDLE FenceEventHandle;
		UINT64 LastSignaledFenceNumber = 0;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> DirectCmdAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;
		Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;
		static const int SwapChainBufferCount = 2;
		Microsoft::WRL::ComPtr<ID3D12Resource> SwapChainBuffer[SwapChainBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource> DepthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DsvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DefaultDescriptorHeap;
		IRootSignatureIPtr SimpleBoxRootSig;
		std::unordered_map<PSOID, Microsoft::WRL::ComPtr<ID3D12PipelineState>> PSOs;
		PSOID NextPSOId = 1;
		HWND WindowHandle = 0;
		UINT RtvDescriptorSize = 0;
		UINT DsvDescriptorSize = 0;
		UINT CbvSrvUavDescriptorSize = 0;
		UINT SamplerDescriptorSize = 0;

		DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		bool      Msaa4xState = false;    // 4X MSAA enabled
		UINT      Msaa4xQuality = 0;      // quality level of 4X MSAA
		int CurrBackBuffer = 0;

		D3D12_VIEWPORT ScreenViewport;
		D3D12_RECT ScissorRect;

	public:
		
		// IRenderer Interfaces
		virtual bool Initialize(void* windowHandle) override;
		virtual void Finalize() override;
		virtual void WaitFence(UINT64 fence) override;
		virtual int GetNumSwapchainBuffers() override;
		//virtual void PrepareDescriptorHeap(EDescriptorHeapType heapType, UINT count) override;
		virtual void OnResized() override;

		virtual ICommandAllocator* CreateCommandAllocator() override;
		virtual IVertexBuffer* CreateVertexBuffer(const void* vertexData, UINT size, UINT stride, bool keepData) override;
		virtual IIndexBuffer* CreateIndexBuffer(const void* indexData, UINT size, EIndexBufferFormat format, bool keepData) override;
		virtual IUploadBuffer* CreateUploadBuffer(UINT elementSize, UINT count, bool constantBuffer) override;
		virtual PSOID CreateGraphicsPipelineState(const FPSODesc& psoDesc) override;
		virtual void DestroyGraphicsPipelineState(PSOID psoid) override;
		virtual IDescriptorHeap* CreateDescriptorHeap(EDescriptorHeapType type, UINT count) override;
		virtual IShader* CompileShader(const wchar_t* filepath, const FShaderMacro* macros,
			EShaderType shaderType, const char* entryFunctionName) override;
		virtual ITexture* LoadTexture(const wchar_t* filepath) override;
		virtual EDataFormat GetBackBufferFormat() const override;
		virtual EDataFormat GetDepthStencilFormat() const override;
		virtual IRootSignature* CreateRootSignature(const char* definition) override;
		virtual int GetSampleCount() const override;
		virtual int GetMsaaQuality() const override;
		virtual int GetBackbufferWidth() const override;
		virtual int GetBackbufferHeight() const override;
		virtual void ResetCommandList(ICommandAllocatorIPtr cmdAllocator, PSOID pso) override;
		virtual void CloseCommandList() override;
		virtual void ExecuteCommandList() override;
		virtual void PresentAndSwapBuffer() override;
		virtual void FlushCommandQueue() override;

		//virtual void BindDescriptorHeap(EDescriptorHeapType type) override;
		virtual void SetGraphicsRootConstantBufferView(int rootParamIndex, fb::IUploadBufferIPtr constantBuffer, int elementIndex) override;
		virtual void SetGraphicsRootDescriptorTable(int rootParamIndex, IDescriptorHeapIPtr descriptorHeap, int index) override;
		virtual void SetGraphicsRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet, const void* pSrcData, UINT DestOffsetIn32BitValues) override;
		virtual void SetPrimitiveTopology(const fb::EPrimitiveTopology topology) override;
		virtual void SetPipelineState(PSOID psoID) override;
		virtual void DrawIndexedInstanced(UINT IndexCountPerInstance,
			UINT InstanceCount,
			UINT StartIndexLocation,
			INT BaseVertexLocation,
			UINT StartInstanceLocation) override;
		virtual void DrawInstanced(UINT VertexCountPerInstance,
			UINT InstanceCount,
			UINT StartVertexLocation,
			UINT StartInstanceLocation) override;
		virtual void ResourceBarrier_Backbuffer_PresentToRenderTarget() override;
		virtual void ResourceBarrier_Backbuffer_RenderTargetToPresent() override;
		virtual void SetViewportAndScissor(int x, int y, UINT width, UINT height) override;
		virtual void ClearRenderTargetDepthStencil(float clearColor[4]) override;
		virtual void SetDefaultRenderTargets() override;
		virtual UINT64 SignalFence() override;
		virtual void SetStencilRef(UINT stencilRef) override;

		virtual void TempResetCommandList() override;
		virtual void TempCreateRootSignatureForSimpleBox() override;
		virtual void TempBindVertexBuffer(const IVertexBufferIPtr& vb) override;
		virtual void TempBindIndexBuffer(const IIndexBufferIPtr& ib) override;		
		virtual void TempDrawIndexedInstanced(UINT indexCount) override;

		// Owning Functions
		Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferInDefaultHeap(
			const void* initData,
			UINT64 byteSize);

		ID3D12Device* GetDevice() const { return Device.Get(); }
		ID3D12DescriptorHeap* GetDefaultDescriptorHeap() const { return DefaultDescriptorHeap.Get(); }
		UINT GetCbvSrvUavDescriptorSize() const { return CbvSrvUavDescriptorSize; }
		void Bind(ID3D12RootSignature* rootSig);
		ID3D12GraphicsCommandList* GetGraphicsCommandList() const { return CommandList.Get(); }
		UINT GetDescriptorHeapStride(EDescriptorHeapType type) const;
		// Add Public Func;		

	private:

		void LogAdapters();
		void LogAdapterOutputs(IDXGIAdapter* adapter);
		void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

		void CreateCommandObjects();
		void CreateSwapChain();
		void CreateRtvAndDsvDescriptorHeaps();

		UINT GetClientWidth() const;
		UINT GetClientHeight() const;

		ID3D12Resource* CurrentBackBuffer()const;
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;

		void BuildDescriptorHeaps();

		// Add Private Func
	};
	extern RendererD3D12* gRendererD3D12;
}

extern "C"
{
	FBRendererD3D12_DLL fb::IRenderer* CreateRendererD3D12();
}
