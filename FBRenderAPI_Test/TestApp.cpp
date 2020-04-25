#include "../FBRenderAPI/RenderAPI.h"

#define VK_MAKE_VERSION(major, minor, patch) \
    (((major) << 22) | ((minor) << 12) | (patch))

using namespace fb;
int main()
{
	InitInfo info;
	info.ApplicationName = "MPGETestApp";
	info.ApplicationVersion = VK_MAKE_VERSION(0, 0, 0);
	info.EngineName = "MPGE";
	info.EngineVersion = VK_MAKE_VERSION(0, 0, 0);
	auto render = RenderAPI::Initialize(eRenderAPIName::DX12, &info);
	if (!render || !render->Success())
	{
		fprintf(stderr, "Failed to create renderer.\n");
		return 1;
	}

	auto GPUs = render->GetGPUs();

	return 0;
}