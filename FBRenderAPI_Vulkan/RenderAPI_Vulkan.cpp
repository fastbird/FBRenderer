#include "stdafx.h"
#include "RenderAPI_Vulkan.h"
#include "VulkanError.inl"

extern "C" {
	fb::RenderAPI* Initialize(fb::RenderAPI::eResult* ret, fb::InitInfo* initInfo)
	{
		if (!initInfo) {
			if (ret) {
				*ret = fb::RenderAPI::eResult::InvalidParameterError;
			}
			return nullptr;
		}
		auto api = fb::MPGEVulkan::Initialize(initInfo);
		if (api && ret) {
			*ret = api->GetLastResult();
		}
		return api;
	}
}

using namespace fb;

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
	FreeLibrary(VulkanModule);
}

bool MPGEVulkan::IsSupportedLayer(const char* layerName) const
{
	static PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties =
		(PFN_vkEnumerateInstanceLayerProperties)vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceLayerProperties");
	if (!vkEnumerateInstanceLayerProperties) {
		LastResult = RenderAPI::eResult::FunctionNotFoundError;
		fprintf(stderr, "vkEnumerateInstanceLayerProperties() is not found.\n");
		return false;
	}
	uint32_t count;
	VkResult result;
	result = vkEnumerateInstanceLayerProperties(&count, nullptr);
	if (result != VK_SUCCESS) {
		LastResult = RenderAPI::eResult::GeneralError;
		fprintf(stderr, "vkEnumerateInstanceLayerProperties() failed.\n");
		return false;
	}
	std::vector<VkLayerProperties> layerProperties(count);
	result = vkEnumerateInstanceLayerProperties(&count, layerProperties.data());
	if (result < 0) {
		LastResult = RenderAPI::eResult::GeneralError;
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
		LastResult = RenderAPI::eResult::FunctionNotFoundError;
		fprintf(stderr, "vkEnumerateInstanceVersion() not found.\n");
		return;
	}

	uint32_t instanceVersion;
	vkEnumerateInstanceVersion(&instanceVersion); // Always return VK_SUCCESS
	uint32_t versionMajor = VK_VERSION_MAJOR(instanceVersion);
	uint32_t versionMinor = VK_VERSION_MINOR(instanceVersion);
	uint32_t versionPatch = VK_VERSION_PATCH(instanceVersion);
	fprintf(stdout, "Vulkan version : %u.%u.%u\n", versionMajor, versionMinor, versionPatch);

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
	try
	{
		vkInst = vk::createInstance(createInfo);
	}
	catch (const vk::SystemError error)
	{
		OnVulkanError(error, LastResult);
		return;
	}
	LastResult = RenderAPI::eResult::Success;
}


std::vector<PhysicalDeviceProperties> MPGEVulkan::GetGPUs() const
{
	LastResult = RenderAPI::eResult::Success;
	if (!vkInst)
	{
		LastResult = RenderAPI::eResult::NotInitializedError;
		fprintf(stderr, "Vulkan Instance is not initialized.\n");
		return {};
	}
	std::vector<PhysicalDeviceProperties> ret;
	auto devices = vkInst.enumeratePhysicalDevices();
	for (auto it : devices)
	{
		auto vkProperties = it.getProperties();
		PhysicalDeviceProperties properties;
		properties.DeviceId = vkProperties.deviceID;
		properties.DriverVersion = vkProperties.driverVersion;
		properties.Name = vkProperties.deviceName;
		properties.VendorId = vkProperties.vendorID;
		switch (properties.VendorId)
		{
		case 0x10DE:
			properties.Vendor = eGPUVendor::Nvidia; break;
		case 0x1002:
			properties.Vendor = eGPUVendor::AMD; break;
		case 0x8086:
			properties.Vendor = eGPUVendor::Intel; break;
		default:
			properties.Vendor = eGPUVendor::Unknown;
		}
		uint64_t memorySize = 0;
		auto memoryProperties = it.getMemoryProperties();
		for (int hi = 0; hi < memoryProperties.memoryHeapCount; ++hi)
		{
			if (memoryProperties.memoryHeaps[hi].flags & vk::MemoryHeapFlagBits::eDeviceLocal)
			{
				memorySize += memoryProperties.memoryHeaps[hi].size;
			}
		}

		properties.DedicatedVideoMemory = memorySize;
		ret.push_back(properties);
	}
	return ret;
}