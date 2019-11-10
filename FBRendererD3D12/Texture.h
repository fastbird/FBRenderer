#pragma once
#include "../ITexture.h"
namespace fb
{
	class Texture : public ITexture
	{
	public:
		Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	};
}