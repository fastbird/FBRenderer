#include "pch.h"
#include "DescriptorHeap.h"
#include "GlobalFunctions.h"
#include "Texture.h"
#include "UploadBuffer.h"
using namespace fb;

bool DescriptorHeap::CreateSRV(UINT heapIndex, ITextureIPtr texture)
{
	if (!DescriptorHeapD3D || !texture)
		return false;

	assert(Type == EDescriptorHeapType::CBV_SRV_UAV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(DescriptorHeapD3D->GetCPUDescriptorHandleForHeapStart());
	if (heapIndex > 0)
	{
		hDescriptor.Offset(heapIndex, GetCbvSrvUavDescriptorSize());
	}

	auto texResource = ((Texture*)texture.get())->Resource.Get();
	if (!texResource)
		return false;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = texResource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = texResource->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	GetDevice()->CreateShaderResourceView(texResource, &srvDesc, hDescriptor);
	return true;
}

bool DescriptorHeap::CreateUAV(UINT heapIndex, ITextureIPtr texture)
{
	auto texResource = ((Texture*)texture.get())->Resource.Get();
	if (!texResource)
		return false;

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(DescriptorHeapD3D->GetCPUDescriptorHandleForHeapStart());
	if (heapIndex > 0)
	{
		hDescriptor.Offset(heapIndex, GetCbvSrvUavDescriptorSize());
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

	uavDesc.Format = texResource->GetDesc().Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	GetDevice()->CreateUnorderedAccessView(texResource, nullptr, &uavDesc, hDescriptor);
}

bool DescriptorHeap::CreateCBV(UINT heapIndex, IUploadBufferIPtr uploadBuffer, UINT elementIndex)
{
	if (!DescriptorHeapD3D)
		return false;
	assert(Type == EDescriptorHeapType::CBV_SRV_UAV);

	UploadBuffer* ub = (UploadBuffer*)uploadBuffer.get();
	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = ub->Resource->GetGPUVirtualAddress();
	auto elementSize = ub->GetElementSize();
	cbAddress += (UINT64)elementIndex * (UINT64)elementSize;
	
	auto hDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapD3D->GetCPUDescriptorHandleForHeapStart());
	if (heapIndex > 0) {
		hDescriptor.Offset(heapIndex, GetCbvSrvUavDescriptorSize());
	}
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = elementSize;
	GetDevice()->CreateConstantBufferView(&cbvDesc, hDescriptor);
	return true;
}

EDescriptorHeapType DescriptorHeap::GetType() const
{
	return Type;
}

void DescriptorHeap::Bind()
{
	ID3D12DescriptorHeap* heaps[] = { DescriptorHeapD3D.Get() };
	GetCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);
}