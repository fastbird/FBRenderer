#pragma once
#include "../ICommandAllocator.h"

namespace fb {
	class CommandAllocator : public ICommandAllocator
	{
		friend class RendererD3D12;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;

	public:
		virtual void Reset() override;
	};
}
