#pragma once
#include "Types.h"
#include "../FBCommon/IRefCounted.h"
namespace fb
{
	enum class EDescriptorHeapType
	{
		CBV_SRV_UAV,
		Sampler,
		RTV,
		DSV
	};

	FBDeclareIntrusivePointer(IUploadBuffer);
	FBDeclareIntrusivePointer(ITexture);
	FBDeclareIntrusivePointer(IDescriptorHeap);
	class IDescriptorHeap : public IRefCounted
	{
	public:
		virtual ~IDescriptorHeap() {}
		virtual bool CreateSRV(UINT heapIndex, ITextureIPtr texture) = 0;
		virtual bool CreateUAV(UINT heapIndex, ITextureIPtr texture) = 0;
		virtual bool CreateCBV(UINT heapIndex, IUploadBufferIPtr uploadBuffer, UINT elementIndex) = 0;
		virtual EDescriptorHeapType GetType() const = 0;
		virtual void Bind() = 0;
	};
}