#pragma once
#include "stdafx.h"

namespace fb
{
	enum class RenderAPIName {
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

	class RenderAPI
	{
	public:
		enum class Result
		{
			Success = 0,
			ModuleNotFound = -1,
			FunctionNotFound = -2,
			GeneralError = -3,
			PlatformError = -4,
			ModuleEntryPointNotFound = -5,
			InvalidParameter = -6,
		};

		MPGE_DLL static Result LastResult;
		MPGE_DLL static RenderAPI* Initialize(RenderAPIName apiName, InitInfo* initInfo);
		virtual void Finalize() = 0;
		
	};
}