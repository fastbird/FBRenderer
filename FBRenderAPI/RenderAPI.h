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
			ModuleNotFoundError = -1,
			FunctionNotFoundError = -2,
			GeneralError = -3,
			PlatformError = -4,
			ModuleEntryPointNotFoundError = -5,
			InvalidParameterError = -6,
			// vk::SystemErrors. Can be shared other API's error.
			OutOfHostMemoryError = -7,
			OutOfDeviceMemoryError = -8,
			InitializationFailedError = -9,
			DeviceLostError = -10,
			MemoryMapFailedError = -11,
			LayerNotPresentError = -12,
			ExtensionNotPresentError = -13,
			FeatureNotPresentError = -14,
			IncompatibleDriverError = -15,
			TooManyObjectsError = -16,
			FormatNotSupportedError = -17,
			FragmentedPoolError = -18,
			UnknownError = -19,
			OutOfPoolMemoryError = -20,
			InvalidExternalHandleError = -21,
			FragmentationError = -22,
			InvalidOpaqueCaptureAddressError = -23,
			SurfaceLostKHRError = -24,
			NativeWindowInUseKHRError = -25,
			OutOfDateKHRError = -26,
			IncompatibleDisplayKHRError = -27,
			ValidationFailedEXTError = -28,
			InvalidShaderNVError = -29,
			InvalidDrmFormatModifierPlaneLayoutEXTError = -30,
			NotPermittedEXTError = -31,
			FullScreenExclusiveModeLostEXTError = -32,
			// vk::SystemError End


		};

		MPGE_DLL static Result LastResult;
		MPGE_DLL static RenderAPI* Initialize(RenderAPIName apiName, InitInfo* initInfo);
		virtual void Finalize() = 0;
		
	};
}