#pragma once
#include "../ITexture.h"
namespace fb
{
	class Texture : public ITexture
	{
		friend class RendererD3D12;
		Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	};
}