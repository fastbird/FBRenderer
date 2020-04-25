#include "stdafx.h"
#include "RenderAPI.h"

namespace fb
{
	RenderAPI::eResult RenderAPI::LastResult;

	RenderAPI* InitDX12(RenderAPI::eResult& LastResult, InitInfo* initInfo)
	{
#if defined(_MSC_VER)
		// Load MPGE_DX12.dll
		auto dx12 = LoadLibrary(L"FBRenderAPI_DX12.dll");
		if (!dx12) {
			LastResult = RenderAPI::eResult::ModuleNotFoundError;
			fprintf(stderr, "FBRenderAPI_DX12.dll not found.\n");
			return nullptr;
		}
		typedef RenderAPI* (*InitializeProc)(RenderAPI::eResult*, InitInfo*);
		InitializeProc Initialize = (InitializeProc)GetProcAddress(dx12, "Initialize");
		if (!Initialize) {
			LastResult = RenderAPI::eResult::ModuleEntryPointNotFoundError;
			fprintf(stderr, "Module entry point function not found.\n");
			return nullptr;
		}
		return Initialize(&LastResult, initInfo);
#else
		MPGE::LastResult = MPGEResult::PlatformError;
		fprintf(stderr, "Invalid platform.\n");
		return nullptr;
#endif
	}

	RenderAPI* InitVulkan(RenderAPI::eResult& LastResult, InitInfo* initInfo)
	{
#if defined(_MSC_VER)
		// Load MPGE_Vulkan.dll
		auto vulkan = LoadLibrary(L"FBRenderAPI_Vulkan.dll");
		if (!vulkan) {
			LastResult = RenderAPI::eResult::ModuleNotFoundError;
			fprintf(stderr, "MPGE_Vulkan.dll not found.\n");
			return nullptr;
		}
		typedef RenderAPI* (*InitializeProc)(RenderAPI::eResult*, InitInfo*);
		InitializeProc Initialize = (InitializeProc)GetProcAddress(vulkan, "Initialize");
		if (!Initialize) {
			LastResult = RenderAPI::eResult::ModuleEntryPointNotFoundError;
			fprintf(stderr, "Module entry point function not found.\n");
			return nullptr;
		}
		return Initialize(&LastResult, initInfo);
#else
		// TODO: Mac, Android, iOS
		MPGE::LastResult = MPGEResult::PlatformError;
		fprintf(stderr, "Invalid platform.\n");
		return nullptr;
#endif
	}

	RenderAPI* InitMetal(RenderAPI::eResult& LastResult, InitInfo* initInfo)
	{
		// TODO : Metal API
		return nullptr;
	}

	MPGE_DLL RenderAPI* RenderAPI::Initialize(eRenderAPIName apiName, InitInfo* initInfo)
	{
		if (!initInfo) {
			LastResult = RenderAPI::eResult::InvalidParameterError;
			fprintf(stderr, "'initInfo' must not be null.\n");
			return nullptr;
		}
		switch (apiName) {
		case eRenderAPIName::DX12:
			return InitDX12(LastResult, initInfo);
		case eRenderAPIName::Vulkan:
			return InitVulkan(LastResult, initInfo);
		case eRenderAPIName::Metal:
			return InitMetal(LastResult, initInfo);
		}
		LastResult = RenderAPI::eResult::InvalidParameterError;
		fprintf(stderr, "Invaild API Name.\n");
		return nullptr;
	}
}