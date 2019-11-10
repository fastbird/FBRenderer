#pragma once


namespace fb
{
	ID3D12Device* GetDevice();
	ID3D12GraphicsCommandList* GetCommandList();
	UINT GetCbvSrvUavDescriptorSize();
}