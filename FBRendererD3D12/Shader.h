#pragma once
#include "../IShader.h"
namespace fb {
	class Shader : public IShader
	{
		friend class RendererD3D12;
		Microsoft::WRL::ComPtr<ID3DBlob> ByteCode;
	public:

	};
}
