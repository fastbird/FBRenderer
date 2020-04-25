#pragma once
#include "stdafx.h"
#include <vector>
#include <string>

namespace fb
{
	enum class eRenderAPIName {
		DX12,
		Vulkan,
		Metal,
	};

	struct InitInfo
	{
		const char* ApplicationName = "App_Name";
		uint32_t ApplicationVersion = 0;
		const char* EngineName = "Engine_Name";
		uint32_t EngineVersion = 0;
	};

	enum class eGPUVendor
	{
		Nvidia,
		AMD,
		Intel,
		Unknown
	};
	struct PhysicalDeviceProperties
	{
		std::string Name;
		eGPUVendor Vendor;
		uint32_t VendorId;
		uint32_t DeviceId;
		uint64_t DriverVersion; // Driver Version has different meaning on different Render API
		uint64_t DedicatedVideoMemory;
	};

	class RenderAPI
	{
		
	public:
#include "RenderAPIError.inl"

	private:
		static eResult LastResult;

	public:
		eResult GetLastResult() const { return LastResult; }
		bool Success() const { return LastResult == eResult::Success; }
		MPGE_DLL static RenderAPI* Initialize(eRenderAPIName apiName, InitInfo* initInfo);
		virtual void Finalize() = 0;

		virtual std::vector<PhysicalDeviceProperties> GetGPUs() const = 0;
		
	};
}