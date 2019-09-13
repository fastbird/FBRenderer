#include  "IRenderer.h"
#include "../FBCommon/Utility.h"

using namespace fb;

void IRenderer::BuildFrameResources()
{
	FrameResources.clear();
	const auto numBuffers = GetNumSwapchainBuffers();
	
	for (int i = 0; i < numBuffers; ++i) {
		FrameResources.push_back(FFrameResource());
		auto& fr = FrameResources.back();
		fr.CommandAllocator = CreateCommandAllocator();
	}	
}

FFrameResource& IRenderer::GetFrameResource_WaitAvailable(UINT index)
{
	auto& fr = FrameResources[index];
	WaitFence(fr.Fence);
	return fr;
}

UINT IRenderer::CalcConstantBufferByteSize(UINT beforeAligned) const
{
	return fb::CalcAligned(beforeAligned, 256);
}