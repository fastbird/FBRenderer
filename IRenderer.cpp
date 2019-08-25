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
		/*fr.CBPerFrame = CreateUploadBuffer(perFrameCBSize, 1, true, EDescriptorHeapType::Default);
		fr.CBPerFrame->CreateCBV(0, EDescriptorHeapType::Default, i * (numObjects + 1));
		fr.CBPerObject = CreateUploadBuffer(perObjectCBSize, numObjects, true, EDescriptorHeapType::Default);
		for (UINT o = 0; o < numObjects; ++o) {
			fr.CBPerObject->CreateCBV(o, EDescriptorHeapType::Default, i * (numObjects + 1) + o);
		}*/
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