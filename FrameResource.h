#pragma once
#include "../FBCommon/Types.h"
#include "Types.h"
#include "ICommandAllocator.h"
#include "EDescriptorHeapType.h"
namespace fb
{
	FBDeclareIntrusivePointer(IUploadBuffer);
	class IUploadBuffer : public IRefCounted
	{
	public:
		virtual ~IUploadBuffer() {}
		// align : 256 for constant buffers. 0 for otherwise.
		virtual bool Initialize(UINT elementSize, UINT align, UINT count) = 0;
		virtual void CopyData(UINT elementIndex, void* elementData) = 0;
		virtual void CreateCBV(UINT elementIndex, EDescriptorHeapType heapType, UINT heapIndex) = 0;
	};

	struct FFrameResource
	{
		ICommandAllocatorIPtr CommandAllocator;
		IUploadBufferIPtr CBPerFrame;
		IUploadBufferIPtr CBPerObject;
		UINT64 Fence = 0;
	};
}
