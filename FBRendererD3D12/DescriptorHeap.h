#pragma once
#include "../IDescriptorHeap.h"
namespace fb
{
	class DescriptorHeap : public IDescriptorHeap
	{
	public:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeapD3D;
		EDescriptorHeapType Type;

		virtual bool CreateDescriptor(UINT heapIndex, ITextureIPtr texture) override;
		virtual bool CreateDescriptor(UINT heapIndex, IUploadBufferIPtr uploadBuffer, UINT elementIndex) override;
		virtual EDescriptorHeapType GetType() const override;
		virtual void Bind() override;
	};
}