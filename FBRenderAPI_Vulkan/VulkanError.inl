static void OnVulkanError(const vk::SystemError& e, fb::RenderAPI::eResult& LastResult)
{
	switch (static_cast<vk::Result>(e.code().value()))
	{
	case vk::Result::eErrorOutOfHostMemory:
		LastResult = fb::RenderAPI::eResult::OutOfHostMemoryError;
		break;
	case vk::Result::eErrorOutOfDeviceMemory:
		LastResult = fb::RenderAPI::eResult::OutOfDeviceMemoryError;
		break;
	case vk::Result::eErrorInitializationFailed:
		LastResult = fb::RenderAPI::eResult::InitializationFailedError;
		break;
	case vk::Result::eErrorDeviceLost:
		LastResult = fb::RenderAPI::eResult::DeviceLostError;
		break;
	case vk::Result::eErrorMemoryMapFailed:
		LastResult = fb::RenderAPI::eResult::MemoryMapFailedError;
		break;
	case vk::Result::eErrorLayerNotPresent:
		LastResult = fb::RenderAPI::eResult::LayerNotPresentError;
		break;
	case vk::Result::eErrorExtensionNotPresent:
		LastResult = fb::RenderAPI::eResult::ExtensionNotPresentError;
		break;
	case vk::Result::eErrorFeatureNotPresent:
		LastResult = fb::RenderAPI::eResult::FeatureNotPresentError;
		break;
	case vk::Result::eErrorIncompatibleDriver:
		LastResult = fb::RenderAPI::eResult::IncompatibleDriverError;
		break;
	case vk::Result::eErrorTooManyObjects:
		LastResult = fb::RenderAPI::eResult::TooManyObjectsError;
		break;
	case vk::Result::eErrorFormatNotSupported:
		LastResult = fb::RenderAPI::eResult::FormatNotSupportedError;
		break;
	case vk::Result::eErrorFragmentedPool:
		LastResult = fb::RenderAPI::eResult::FragmentedPoolError;
		break;
	case vk::Result::eErrorUnknown:
		LastResult = fb::RenderAPI::eResult::UnknownError;
		break;
	case vk::Result::eErrorOutOfPoolMemory:
		LastResult = fb::RenderAPI::eResult::OutOfPoolMemoryError;
		break;
	case vk::Result::eErrorInvalidExternalHandle:
		LastResult = fb::RenderAPI::eResult::InvalidExternalHandleError;
		break;
	}
	fprintf(stderr, "%s Error : %s\n"
		"\tErrorCode : %d\n"
		"\tErrorMessage : %s\n"
		, e.code().category().name(), e.what(),
		e.code().value(),
		e.code().message().c_str());
}