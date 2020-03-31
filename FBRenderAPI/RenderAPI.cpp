#include "stdafx.h"
#include "RenderAPI.h"

namespace fb
{
	MPGE_DLL RenderAPI::Result RenderAPI::LastResult;

	RenderAPI* InitDX12(InitInfo* initInfo)
	{
#if defined(_MSC_VER)
		// Load MPGE_DX12.dll
		auto dx12 = LoadLibrary(L"FBRenderAPI_DX12.dll");
		if (!dx12) {
			RenderAPI::LastResult = RenderAPI::Result::ModuleNotFound;
			fprintf(stderr, "FBRenderAPI_DX12.dll not found.\n");
			return nullptr;
		}
		typedef RenderAPI* (*InitializeProc)(RenderAPI::Result*, InitInfo*);
		InitializeProc Initialize = (InitializeProc)GetProcAddress(dx12, "Initialize");
		if (!Initialize) {
			RenderAPI::LastResult = RenderAPI::Result::ModuleEntryPointNotFound;
			fprintf(stderr, "Module entry point function not found.\n");
			return nullptr;
		}
		return Initialize(&RenderAPI::LastResult, initInfo);
#else
		MPGE::LastResult = MPGEResult::PlatformError;
		fprintf(stderr, "Invalid platform.\n");
		return nullptr;
#endif
	}

	RenderAPI* InitVulkan(InitInfo* initInfo)
	{
#if defined(_MSC_VER)
		// Load MPGE_Vulkan.dll
		auto vulkan = LoadLibrary(L"FBRenderAPI_Vulkan.dll");
		if (!vulkan) {
			RenderAPI::LastResult = RenderAPI::Result::ModuleNotFound;
			fprintf(stderr, "MPGE_Vulkan.dll not found.\n");
			return nullptr;
		}
		typedef RenderAPI* (*InitializeProc)(RenderAPI::Result*, InitInfo*);
		InitializeProc Initialize = (InitializeProc)GetProcAddress(vulkan, "Initialize");
		if (!Initialize) {
			RenderAPI::LastResult = RenderAPI::Result::ModuleEntryPointNotFound;
			fprintf(stderr, "Module entry point function not found.\n");
			return nullptr;
		}
		return Initialize(&RenderAPI::LastResult, initInfo);
#else
		// TODO: Mac, Android, iOS
		MPGE::LastResult = MPGEResult::PlatformError;
		fprintf(stderr, "Invalid platform.\n");
		return nullptr;
#endif
	}

	RenderAPI* InitMetal(InitInfo* initInfo)
	{
		// TODO : Metal API
		return nullptr;
	}

	MPGE_DLL RenderAPI* RenderAPI::Initialize(RenderAPIName apiName, InitInfo* initInfo)
	{
		if (!initInfo) {
			LastResult = RenderAPI::Result::InvalidParameter;
			fprintf(stderr, "'initInfo' must not be null.\n");
			return nullptr;
		}
		switch (apiName) {
		case RenderAPIName::DX12:
			return InitDX12(initInfo);
		case RenderAPIName::Vulkan:
			return InitVulkan(initInfo);
		case RenderAPIName::Metal:
			return InitMetal(initInfo);
		}
		LastResult = RenderAPI::Result::InvalidParameter;
		fprintf(stderr, "Invaild API Name.\n");
		return nullptr;
	}
}