enum class eResult
{
	Success = 0,
	ModuleNotFoundError = -1,
	FunctionNotFoundError = -2,
	GeneralError = -3,
	PlatformError = -4,
	ModuleEntryPointNotFoundError = -5,
	InvalidParameterError = -6, // == invalid call
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

	NotInitializedError = -100,

	// DXGIError Start
	AccessDeniedError = -101,
	AccessLostError = -102,
	AlreadyExistsError = -103,
	CannotProtectContentError = -104,
	DeviceHungError = -105,
	DeviceRemovedError = -106,
	DeviceResetError = -107,
	DriverInternalError = -108,
	FrameStatisticsDisjointError = -109,
	GraphicsVidpnSourceInUseError = -110,
	MoreDataError = -111,
	NameAlreadyExistsError = -112,
	NonExclusiveError = -113,
	NotCurrentlyAvailableError = -114,
	GUIDNotFoundError = -115,
	RemoteClientDisconnectedError = -116,
	RemoteOutOfMemoryError = -117,
	RestrictToOuputStaleError = -118,
	SDKComponentMissingError = -119,
	SessionDisconnedtedError = -120,
	UnsupportedError = -121,
	WaitTimeoutError = -122,
	WasStillDrawingError = -123,
	// DXGI End
};

const char* ToString(const eResult ret)
{
	switch (ret)
	{
	case	eResult::Success: return "Success";
	case	eResult::ModuleNotFoundError: return "ModuleNotFoundError";
	case	eResult::FunctionNotFoundError: return "FunctionNotFoundError";
	case	eResult::GeneralError: return "GeneralError";
	case	eResult::PlatformError: return "PlatformError";
	case	eResult::ModuleEntryPointNotFoundError: return "ModuleEntryPointNotFoundError";
	case	eResult::InvalidParameterError: return "InvalidParameterError";
		
	case	eResult::OutOfHostMemoryError: return "OutOfHostMemoryError";
	case	eResult::OutOfDeviceMemoryError: return "OutOfDeviceMemoryError";
	case	eResult::InitializationFailedError: return "InitializationFailedError";
	case	eResult::DeviceLostError: return "DeviceLostError";
	case	eResult::MemoryMapFailedError: return "MemoryMapFailedError";
	case	eResult::LayerNotPresentError: return "LayerNotPresentError";
	case	eResult::ExtensionNotPresentError: return "ExtensionNotPresentError";
	case	eResult::FeatureNotPresentError: return "FeatureNotPresentError";
	case	eResult::IncompatibleDriverError: return "IncompatibleDriverError";
	case	eResult::TooManyObjectsError: return "TooManyObjectsError";
	case	eResult::FormatNotSupportedError: return "FormatNotSupportedError";
	case	eResult::FragmentedPoolError: return "FragmentedPoolError";
	case	eResult::UnknownError: return "UnknownError";
	case	eResult::OutOfPoolMemoryError: return "OutOfPoolMemoryError";
	case	eResult::InvalidExternalHandleError: return "InvalidExternalHandleError";
	case	eResult::FragmentationError: return "FragmentationError";
	case	eResult::InvalidOpaqueCaptureAddressError: return "InvalidOpaqueCaptureAddressError";
	case	eResult::SurfaceLostKHRError: return "SurfaceLostKHRError";
	case	eResult::NativeWindowInUseKHRError: return "NativeWindowInUseKHRError";
	case	eResult::OutOfDateKHRError: return "OutOfDateKHRError";
	case	eResult::IncompatibleDisplayKHRError: return "IncompatibleDisplayKHRError";
	case	eResult::ValidationFailedEXTError: return "ValidationFailedEXTError";
	case	eResult::InvalidShaderNVError: return "InvalidShaderNVError";
	case	eResult::InvalidDrmFormatModifierPlaneLayoutEXTError: return "InvalidDrmFormatModifierPlaneLayoutEXTError";
	case	eResult::NotPermittedEXTError: return "NotPermittedEXTError";
	case	eResult::FullScreenExclusiveModeLostEXTError: return "FullScreenExclusiveModeLostEXTError";
		
	case	eResult::NotInitializedError: return "NotInitializedError";
		
		
	case	eResult::AccessDeniedError: return "AccessDeniedError";
	case	eResult::AccessLostError: return "AccessLostError";
	case	eResult::AlreadyExistsError: return "AlreadyExistsError";
	case	eResult::CannotProtectContentError: return "CannotProtectContentError";
	case	eResult::DeviceHungError: return "DeviceHungError";
	case	eResult::DeviceRemovedError: return "DeviceRemovedError";
	case	eResult::DeviceResetError: return "DeviceResetError";
	case	eResult::DriverInternalError: return "DriverInternalError";
	case	eResult::FrameStatisticsDisjointError: return "FrameStatisticsDisjointError";
	case	eResult::GraphicsVidpnSourceInUseError: return "GraphicsVidpnSourceInUseError";
	case	eResult::MoreDataError: return "MoreDataError";
	case	eResult::NameAlreadyExistsError: return "NameAlreadyExistsError";
	case	eResult::NonExclusiveError: return "NonExclusiveError";
	case	eResult::NotCurrentlyAvailableError: return "NotCurrentlyAvailableError";
	case	eResult::GUIDNotFoundError: return "GUIDNotFoundError";
	case	eResult::RemoteClientDisconnectedError: return "RemoteClientDisconnectedError";
	case	eResult::RemoteOutOfMemoryError: return "RemoteOutOfMemoryError";
	case	eResult::RestrictToOuputStaleError: return "RestrictToOuputStaleError";
	case	eResult::SDKComponentMissingError: return "SDKComponentMissingError";
	case	eResult::SessionDisconnedtedError: return "SessionDisconnedtedError";
	case	eResult::UnsupportedError: return "UnsupportedError";
	case	eResult::WaitTimeoutError: return "WaitTimeoutError";
	case	eResult::WasStillDrawingError: return "WasStillDrawingError";
	default:
		return "No string for the error.";
	}
}