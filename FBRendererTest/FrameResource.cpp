#include "pch.h"
#include "FrameResource.h"
#include "../IRenderer.h"

std::vector<FFrameResource> FrameResources;
extern fb::IRenderer* gRenderer;

void BuildFrameResources()
{
	assert(gRenderer);
	FrameResources.clear();
	const auto numBuffers = gRenderer->GetNumSwapchainBuffers();

	for (int i = 0; i < numBuffers; ++i) {
		FrameResources.push_back(FFrameResource());
		auto& fr = FrameResources.back();
		fr.CommandAllocator = gRenderer->CreateCommandAllocator();
	}
}

FFrameResource& GetFrameResource_WaitAvailable(UINT index)
{
	assert(gRenderer);
	auto& fr = FrameResources[index];
	gRenderer->WaitFence(fr.Fence);
	return fr;
}