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

void DestroyFrameResources()
{
	FrameResources.clear();
}

void BuildConstantBuffers(int numObj, int numMaterials, int numPassCB)
{
	const auto numBuffers = gRenderer->GetNumSwapchainBuffers();
	for (int i = 0; i < numBuffers; ++i) {
		auto& fr = FrameResources[i];
		fr.CBPerObject = gRenderer->CreateUploadBuffer(sizeof(ObjectConstants), numObj, true);
		fr.CBPerMaterial = gRenderer->CreateUploadBuffer(sizeof(MaterialConstants), numMaterials, true);
		fr.CBPerFrame = gRenderer->CreateUploadBuffer(sizeof(PassConstants), numPassCB, true);
	}
}

FFrameResource& GetFrameResource_WaitAvailable(UINT index)
{
	assert(gRenderer);
	auto& fr = FrameResources[index];
	gRenderer->WaitFence(fr.Fence);
	return fr;
}