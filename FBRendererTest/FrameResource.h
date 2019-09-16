#pragma once
#include "../ICommandAllocator.h"
#include "../IUploadBuffer.h"
#include "../Types.h"

struct FFrameResource
{
	fb::ICommandAllocatorIPtr CommandAllocator;
	fb::IUploadBufferIPtr CBPerFrame;
	fb::IUploadBufferIPtr CBPerObject;
	fb::IUploadBufferIPtr WavesVB;
	fb::UINT64 Fence = 0;
};
extern std::vector<FFrameResource> FrameResources;

void BuildFrameResources();
inline FFrameResource& GetFrameResource(UINT index) { return FrameResources[index]; }
FFrameResource& GetFrameResource_WaitAvailable(UINT index);