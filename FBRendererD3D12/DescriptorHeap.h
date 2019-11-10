#pragma once
#include "../IDescriptorHeap.h"
namespace fb
{
	class DescriptorHeap : public IDescriptorHeap
	{
	public:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeap;
		EDescriptorHeapType Type;

		virtual bool CreateDescriptor(UINT index, ITextureIPtr texture) override;
		virtual EDescriptorHeapType GetType() const override;
	};
}