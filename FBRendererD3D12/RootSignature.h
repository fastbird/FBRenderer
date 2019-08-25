#pragma once
#include "../IRootSignature.h"

namespace fb {
	class RootSignature : public IRootSignature
	{
		Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;
	};
}
