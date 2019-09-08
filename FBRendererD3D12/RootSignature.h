#pragma once
#include "../IRootSignature.h"

namespace fb {
	class RootSignature : public IRootSignature
	{
		friend class RendererD3D12;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;

	public:
		virtual void Bind() override;
	};
}
