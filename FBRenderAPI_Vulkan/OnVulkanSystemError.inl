static void OnVulkanSystemError(const vk::SystemError& e, fb::RenderAPI::Result& LastResult)
{
	switch (static_cast<vk::Result>(e.code().value()))
	{
	case vk::Result::eErrorOutOfHostMemory:
		LastResult = fb::RenderAPI::Result::OutOfHostMemoryError;
		break;
	case vk::Result::eErrorOutOfDeviceMemory:
		LastResult = fb::RenderAPI::Result::OutOfDeviceMemoryError;
		break;
	case vk::Result::eErrorInitializationFailed:
		LastResult = fb::RenderAPI::Result::InitializationFailedError;
		break;
	case vk::Result::eErrorDeviceLost:
		LastResult = fb::RenderAPI::Result::DeviceLostError;
		break;
	case vk::Result::eErrorMemoryMapFailed:
		LastResult = fb::RenderAPI::Result::MemoryMapFailedError;
		break;
	case vk::Result::eErrorLayerNotPresent:
		LastResult = fb::RenderAPI::Result::LayerNotPresentError;
		break;
	case vk::Result::eErrorExtensionNotPresent:
		LastResult = fb::RenderAPI::Result::ExtensionNotPresentError;
		break;
	case vk::Result::eErrorFeatureNotPresent:
		LastResult = fb::RenderAPI::Result::FeatureNotPresentError;
		break;
	case vk::Result::eErrorIncompatibleDriver:
		LastResult = fb::RenderAPI::Result::IncompatibleDriverError;
		break;
	case vk::Result::eErrorTooManyObjects:
		LastResult = fb::RenderAPI::Result::TooManyObjectsError;
		break;
	case vk::Result::eErrorFormatNotSupported:
		LastResult = fb::RenderAPI::Result::FormatNotSupportedError;
		break;
	case vk::Result::eErrorFragmentedPool:
		LastResult = fb::RenderAPI::Result::FragmentedPoolError;
		break;
	case vk::Result::eErrorUnknown:
		LastResult = fb::RenderAPI::Result::UnknownError;
		break;
	case vk::Result::eErrorOutOfPoolMemory:
		LastResult = fb::RenderAPI::Result::OutOfPoolMemoryError;
		break;
	case vk::Result::eErrorInvalidExternalHandle:
		LastResult = fb::RenderAPI::Result::InvalidExternalHandleError;
		break;
	}
	fprintf(stderr, "%s Error : %s\n"
		"\tErrorCode : %d\n"
		"\tErrorMessage : %s\n"
		, e.code().category().name(), e.what(),
		e.code().value(),
		e.code().message().c_str());
}