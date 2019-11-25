#pragma once
#include "../IDescriptorHeap.h"
namespace fb
{
	class DescriptorHeap : public IDescriptorHeap
	{
	public:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeapD3D;
		EDescriptorHeapType Type;

		virtual bool CreateSRV(UINT heapIndex, ITextureIPtr texture) override;
		virtual bool CreateUAV(UINT heapIndex, ITextureIPtr texture) override;
		virtual bool CreateCBV(UINT heapIndex, IUploadBufferIPtr uploadBuffer, UINT elementIndex) override;
		virtual EDescriptorHeapType GetType() const override;
		virtual void Bind() override;
	};
}