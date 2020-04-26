#include "../FBRenderAPI/RenderAPI.h"

#define VK_MAKE_VERSION(major, minor, patch) \
    (((major) << 22) | ((minor) << 12) | (patch))

using namespace fb;

int SelectBestGPU(const std::vector<PhysicalDeviceProperties>& Gpus)
{
	int index = -1;
	uint64_t bestMemory = 0;
	for (int i = Gpus.size() - 1; i >=0 ; --i)
	{
		if (Gpus[i].DedicatedVideoMemory >= bestMemory)
		{
			bestMemory = Gpus[i].DedicatedVideoMemory;
			index = i;
		}
	}
	return index;
}

int main()
{
	InitInfo info;
	info.ApplicationName = "MPGETestApp";
	info.ApplicationVersion = VK_MAKE_VERSION(0, 0, 0);
	info.EngineName = "MPGE";
	info.EngineVersion = VK_MAKE_VERSION(0, 0, 0);
	auto render = RenderAPI::Initialize(eRenderAPIName::Vulkan, &info);
	if (!render || !render->Success())
	{
		fprintf(stderr, "Failed to create renderer.\n");
		return 1;
	}

	auto GPUs = render->GetGPUs();
	auto gpuIndex = SelectBestGPU(GPUs);
	if (gpuIndex == -1) {
		fprintf(stderr, "No GPU selected.\n");
		return 2;
	}
	auto device = render->CreateDevice(gpuIndex);

	return 0;
}