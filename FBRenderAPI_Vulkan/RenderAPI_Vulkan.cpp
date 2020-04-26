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

static const char* const RenderDocLayerName = "VK_LAYER_RENDERDOC_Capture";
static const char* const Win32SurfaceKHRExtName = "VK_KHR_win32_surface";
static const char* const ValidationLayerName = "VK_LAYER_KHRONOS_validation";

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

bool IsSupportedInstanceLayer(const char* layerName)
{
	auto properties = vk::enumerateInstanceLayerProperties();
	for (auto& it : properties)
	{
		if (_stricmp(layerName, it.layerName) == 0)
			return true;
	}
	return false;
}

bool IsSupportedInstanceExtension(const char* extensionName)
{
	auto extentionProps = vk::enumerateInstanceExtensionProperties();
	for (auto& prop : extentionProps)
	{
		if (_stricmp(prop.extensionName, extensionName) == 0) {
			return true;
		}
	}
	return false;
}

void MPGEVulkan::CreateInstance(InitInfo* initInfo)
{
	PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion = 
		(PFN_vkEnumerateInstanceVersion)vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion");
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
	auto k2 = vk::enumerateInstanceLayerProperties();
	auto k3 = vk::enumerateInstanceExtensionProperties();
	auto k = vk::enumerateInstanceExtensionProperties(std::string());
#ifdef _DEBUG
	if (IsSupportedInstanceLayer(ValidationLayerName))
	{
		layerNames.push_back(ValidationLayerName);
		VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT };
		VkValidationFeaturesEXT features = {};
		features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
		features.enabledValidationFeatureCount = 1;
		features.pEnabledValidationFeatures = enables;
		createInfo.pNext = &features;
		
	}
	if (IsSupportedInstanceLayer(RenderDocLayerName))
	{
		layerNames.push_back(RenderDocLayerName);
	}
#endif // _DEBUG
	std::vector<const char*> extensionNames;	
	if (IsSupportedInstanceExtension(Win32SurfaceKHRExtName))
	{
		extensionNames.push_back(Win32SurfaceKHRExtName);
	}
	createInfo.enabledLayerCount = (uint32_t)layerNames.size();
	createInfo.ppEnabledLayerNames = layerNames.empty() ? nullptr : layerNames.data();
	createInfo.enabledExtensionCount = (uint32_t)extensionNames.size();
	createInfo.ppEnabledExtensionNames = extensionNames.data();
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
		for (uint32_t hi = 0; hi < memoryProperties.memoryHeapCount; ++hi)
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

bool IsSupportedDeviceLayer(vk::PhysicalDevice& device, const char* layerName)
{
	auto layerProps = device.enumerateDeviceLayerProperties();
	for (auto& prop : layerProps)
	{
		if (_stricmp(prop.layerName, layerName) == 0)
			return true;
	}
	return false;
}

bool IsSupportedDeviceExtension(vk::PhysicalDevice& device, const char* extName)
{
	auto extProps = device.enumerateDeviceExtensionProperties();
	for (auto& prop : extProps)
	{
		if (_stricmp(prop.extensionName, extName) == 0)
			return true;
	}
	return false;
}

RenderAPI::Device* MPGEVulkan::CreateDevice(uint32_t gpuIndex)
{
	LastResult = RenderAPI::eResult::Success;
	if (!vkInst)
	{
		LastResult = RenderAPI::eResult::NotInitializedError;
		fprintf(stderr, "Vulkan Instance is not initialized.\n");
		return nullptr;
	}

	if (Device)
	{
		LastResult = RenderAPI::eResult::AlreadyExistsError;
		fprintf(stderr, "Vulkan Device already exists.\n");
		return nullptr;
	}

	
	auto devices = vkInst.enumeratePhysicalDevices();
	if (gpuIndex >= devices.size())
	{
		LastResult = RenderAPI::eResult::InvalidParameterError;
		fprintf(stderr, "Invalid gpuIndex.\n");
		return nullptr;
	}
	try {
		auto& gpu = devices[gpuIndex];
		auto queueFamilyProperties = gpu.getQueueFamilyProperties();
		auto memoryProperties = gpu.getMemoryProperties();
		auto deviceProperties = gpu.getProperties();
		uint32_t queueFamilyIndex = -1;
		uint32_t index = 0;
		for (auto& it : queueFamilyProperties)
		{
			if (queueFamilyIndex == -1 && it.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				queueFamilyIndex = index;
			}
			if (it.queueFlags & vk::QueueFlagBits::eGraphics && it.queueFlags & vk::QueueFlagBits::eCompute)
			{
				queueFamilyIndex = index;
			}
			++index;
		}

		// The ppEnabledLayerNamesand enabledLayerCount members of VkDeviceCreateInfo are deprecated
		// and their values must be ignored by implementations.However, for compatibility, only an empty list of layers 
		// or a list that exactly matches the sequence enabled at instance creation time are valid, 
		// and validation layers should issue diagnostics for other cases.

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = { vk::DeviceQueueCreateInfo(
			vk::DeviceQueueCreateFlags(),
			queueFamilyIndex
		) };

		std::vector<const char*> enabledLayerNames;
		std::vector<const char*> enabledExtNames;

#ifdef _DEBUG
		if (IsSupportedDeviceLayer(gpu, ValidationLayerName))
		{
			enabledLayerNames.push_back(ValidationLayerName);
		}
		if (IsSupportedDeviceLayer(gpu, RenderDocLayerName))
		{
			enabledLayerNames.push_back(RenderDocLayerName);
		}
#endif
		if (IsSupportedDeviceExtension(gpu, Win32SurfaceKHRExtName))
		{
			enabledExtNames.push_back(Win32SurfaceKHRExtName);
		}
		auto features = gpu.getFeatures();
		// robust buffer access affects performance.
		//features.robustBufferAccess = false;
		vk::DeviceCreateInfo info(
			vk::DeviceCreateFlags(),
			(uint32_t)queueCreateInfos.size(),
			queueCreateInfos.data(),
			(uint32_t)enabledLayerNames.size(),
			enabledLayerNames.data(),
			(uint32_t)enabledExtNames.size(),
			enabledExtNames.data(),
			&features);

		auto device = gpu.createDevice(info);
		Device = new DeviceVulkan(device);
		return Device;
	}
	catch (const vk::SystemError error)
	{
		OnVulkanError(error, LastResult);
		return nullptr;
	}

	return nullptr;
}

DeviceVulkan::DeviceVulkan(vk::Device device)
	: Device(device)
{

}