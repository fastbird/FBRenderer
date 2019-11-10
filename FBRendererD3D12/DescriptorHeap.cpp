#include "pch.h"
#include "DescriptorHeap.h"
#include "GlobalFunctions.h"
#include "Texture.h"
using namespace fb;

bool DescriptorHeap::CreateDescriptor(UINT index, ITextureIPtr texture)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	if (index > 0)
	{
		hDescriptor.Offset(index, GetCbvSrvUavDescriptorSize());
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

EDescriptorHeapType DescriptorHeap::GetType() const
{
	return Type;
}