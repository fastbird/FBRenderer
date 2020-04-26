#pragma once
#include "../FBRenderAPI/RenderAPI.h"

extern "C" {
	MPGE_VULKAN_DLL fb::RenderAPI* Initialize(fb::RenderAPI::eResult* ret, fb::InitInfo* initInfo);
}

namespace fb
{
	class DeviceVulkan : public RenderAPI::Device
	{
	public:
		DeviceVulkan(vk::Device device);

		vk::Device Device;
	};
	
	class MPGEVulkan : public RenderAPI
	{
		MPGEVulkan(InitInfo* initInfo);
		~MPGEVulkan();

		mutable RenderAPI::eResult LastResult = RenderAPI::eResult::Success;
		HMODULE VulkanModule;
		vk::Instance vkInst;
		DeviceVulkan* Device = nullptr;

	public:
		static MPGEVulkan* Initialize(InitInfo* initInfo);
		void Finalize() override;

		void CreateInstance(InitInfo* initInfo);
		RenderAPI::eResult GetLastResult() const { return LastResult; }

		std::vector<PhysicalDeviceProperties> GetGPUs() const override;
		RenderAPI::Device* CreateDevice(uint32_t gpuIndex) override;
		// public functions end	
		
	};
}