#include "pch.h"
#include "GlobalFunctions.h"
#include "RendererD3D12.h"
namespace fb
{
	ID3D12Device* GetDevice()
	{
		return gRendererD3D12->GetDevice();
	}
	ID3D12GraphicsCommandList* GetCommandList()
	{
		return gRendererD3D12->GetGraphicsCommandList();
	}

	UINT GetCbvSrvUavDescriptorSize()
	{
		return gRendererD3D12->GetCbvSrvUavDescriptorSize();
	}
}