static void OnD3DError(const HRESULT hr, fb::RenderAPI::eResult& LastResult)
{
	switch (hr)
	{
	case DXGI_ERROR_ACCESS_DENIED:
		LastResult = fb::RenderAPI::eResult::AccessDeniedError; break;
	case DXGI_ERROR_ACCESS_LOST:
		LastResult = fb::RenderAPI::eResult::AccessLostError; break;
	case DXGI_ERROR_ALREADY_EXISTS:
		LastResult = fb::RenderAPI::eResult::AlreadyExistsError; break;
	case DXGI_ERROR_CANNOT_PROTECT_CONTENT:
		LastResult = fb::RenderAPI::eResult::CannotProtectContentError; break;
	case DXGI_ERROR_DEVICE_HUNG:
		LastResult = fb::RenderAPI::eResult::DeviceHungError; break;
	case DXGI_ERROR_DEVICE_REMOVED:
		LastResult = fb::RenderAPI::eResult::DeviceRemovedError; break;
	case DXGI_ERROR_DEVICE_RESET:
		LastResult = fb::RenderAPI::eResult::DeviceResetError; break;
	case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
		LastResult = fb::RenderAPI::eResult::DriverInternalError; break;
	case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:
		LastResult = fb::RenderAPI::eResult::FrameStatisticsDisjointError; break;
	case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE:
		LastResult = fb::RenderAPI::eResult::GraphicsVidpnSourceInUseError; break;
	case DXGI_ERROR_INVALID_CALL:
		LastResult = fb::RenderAPI::eResult::InvalidParameterError; break;
	case DXGI_ERROR_MORE_DATA:
		LastResult = fb::RenderAPI::eResult::MoreDataError; break;
	case DXGI_ERROR_NAME_ALREADY_EXISTS:
		LastResult = fb::RenderAPI::eResult::NameAlreadyExistsError; break;
	case DXGI_ERROR_NONEXCLUSIVE:
		LastResult = fb::RenderAPI::eResult::NonExclusiveError; break;
	case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
		LastResult = fb::RenderAPI::eResult::NotCurrentlyAvailableError; break;
	case DXGI_ERROR_NOT_FOUND:
		LastResult = fb::RenderAPI::eResult::GUIDNotFoundError; break;
	case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED:
		LastResult = fb::RenderAPI::eResult::RemoteClientDisconnectedError; break;
	case DXGI_ERROR_REMOTE_OUTOFMEMORY:
		LastResult = fb::RenderAPI::eResult::RemoteOutOfMemoryError; break;
	case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE:
		LastResult = fb::RenderAPI::eResult::RestrictToOuputStaleError; break;
	case DXGI_ERROR_SDK_COMPONENT_MISSING:
		LastResult = fb::RenderAPI::eResult::SDKComponentMissingError; break;
	case DXGI_ERROR_SESSION_DISCONNECTED:
		LastResult = fb::RenderAPI::eResult::SessionDisconnedtedError; break;
	case DXGI_ERROR_UNSUPPORTED:
		LastResult = fb::RenderAPI::eResult::UnsupportedError; break;
	case DXGI_ERROR_WAIT_TIMEOUT:
		LastResult = fb::RenderAPI::eResult::WaitTimeoutError; break;
	case DXGI_ERROR_WAS_STILL_DRAWING:
		LastResult = fb::RenderAPI::eResult::WasStillDrawingError; break;
	default:
		LastResult = fb::RenderAPI::eResult::UnknownError;
	}
	_com_error err(hr);	
	fwprintf(stderr, L"Error HRESULT : 0x%x\nError Message: %s\n", hr, err.ErrorMessage());
}