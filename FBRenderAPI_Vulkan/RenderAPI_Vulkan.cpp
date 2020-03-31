#include "stdafx.h"
#include "RenderAPI_Vulkan.h"

extern "C" {
	fb::RenderAPI* Initialize(fb::RenderAPI::Result* ret, fb::InitInfo* initInfo)
	{
		if (!initInfo) {
			if (ret) {
				*ret = fb::RenderAPI::Result::InvalidParameter;
			}
			return nullptr;
		}
		auto vul = fb::MPGEVulkan::Initialize(initInfo);
		if (vul && ret) {
			*ret = vul->GetLastResult();
		}
		return vul;
	}
}

namespace fb {
	MPGEVulkan* MPGEVulkan::Initialize(InitInfo* initInfo)
	{
		auto vul = new MPGEVulkan(initInfo);
		return vul;
	}

	void MPGEVulkan::Finalize()
	{
		delete this;
	}

	MPGEVulkan::MPGEVulkan(InitInfo* initInfo)
	{
		CreateInstance(initInfo);
	}

	MPGEVulkan::~MPGEVulkan()
	{

	}

	bool MPGEVulkan::IsSupportedLayer(const char* layerName) const
	{
		static PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties =
			(PFN_vkEnumerateInstanceLayerProperties)vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceLayerProperties");
		if (!vkEnumerateInstanceLayerProperties) {
			fprintf(stderr, "vkEnumerateInstanceLayerProperties() is not found.\n");
			return false;
		}
		uint32_t count;
		VkResult result;
		result = vkEnumerateInstanceLayerProperties(&count, nullptr);
		if (result != VK_SUCCESS) {
			fprintf(stderr, "vkEnumerateInstanceLayerProperties() failed.\n");
			return false;
		}
		std::vector<VkLayerProperties> layerProperties(count);
		result = vkEnumerateInstanceLayerProperties(&count, layerProperties.data());
		if (result < 0) {
			fprintf(stderr, "vkEnumerateInstanceLayerProperties returned with error code: %d\n", result);
			return false;
		}
		for (const auto& layer : layerProperties)
		{
			if (_stricmp(layer.layerName, layerName) == 0) {
				return true;
			}
			
		}
		return false;
	}

	void MPGEVulkan::CreateInstance(InitInfo* initInfo)
	{
		PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion");
		if (!vkEnumerateInstanceVersion) {
			LastResult = RenderAPI::Result::FunctionNotFound;
			fprintf(stderr, "vkEnumerateInstanceVersion() not found.\n");
			return;
		}

		uint32_t instanceVersion;
		vkEnumerateInstanceVersion(&instanceVersion); // Always return VK_SUCCESS
		uint32_t versionMajor = VK_VERSION_MAJOR(instanceVersion);
		uint32_t versionMinor = VK_VERSION_MINOR(instanceVersion);
		uint32_t versionPatch = VK_VERSION_PATCH(instanceVersion);
		fprintf(stdout, "Vulkan version : %u.%u.%u\n", versionMajor, versionMinor, versionPatch);
		PFN_vkCreateInstance vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(nullptr, "vkCreateInstance");
		if (!vkCreateInstance) {
			LastResult = RenderAPI::Result::FunctionNotFound;
			fprintf(stderr, "vkCreateInstance() not found.\n");
			return;
		}

		vk::ApplicationInfo appInfo;
		appInfo.pApplicationName = initInfo->ApplicationName;
		appInfo.applicationVersion = initInfo->ApplicationVersion;
		appInfo.pEngineName = initInfo->EngineName;
		appInfo.engineVersion = initInfo->EngineVersion;
		appInfo.apiVersion = instanceVersion;
		vk::InstanceCreateInfo createInfo;
		createInfo.setPApplicationInfo(&appInfo);
		std::vector<const char*> layerNames;
		if (IsSupportedLayer("VK_LAYER_KHRONOS_validation"))
		{
			layerNames.push_back("VK_LAYER_KHRONOS_validation");
		}
		createInfo.enabledLayerCount = (uint32_t)layerNames.size();
		createInfo.ppEnabledLayerNames = layerNames.data();
		createInfo.enabledExtensionCount = 0;
		createInfo.ppEnabledExtensionNames = nullptr;
		//vkCreateInstance();
		LastResult = RenderAPI::Result::Success;
	}
}