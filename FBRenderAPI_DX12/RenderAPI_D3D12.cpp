#include "stdafx.h"
#include "RenderAPI_D3D12.h"
#include "D3DError.inl"
extern "C" {
	fb::RenderAPI* Initialize(fb::RenderAPI::eResult* ret, fb::InitInfo* initInfo)
	{
		if (!initInfo) {
			if (ret) {
				*ret = fb::RenderAPI::eResult::InvalidParameterError;
			}
			return nullptr;
		}
		auto api = fb::MPGEDirect3D12::Initialize(initInfo);
		if (api && ret) {
			*ret = api->GetLastResult();
		}
		return api;
	}
}

using namespace fb;

MPGEDirect3D12* MPGEDirect3D12::Initialize(InitInfo* initInfo)
{
	auto vul = new MPGEDirect3D12(initInfo);
	return vul;
}

void MPGEDirect3D12::Finalize()
{
	delete this;
}

MPGEDirect3D12::MPGEDirect3D12(InitInfo* initInfo)
{
	CreateInstance(initInfo);
}

MPGEDirect3D12::~MPGEDirect3D12()
{

}

void MPGEDirect3D12::CreateInstance(InitInfo* initInfo)
{
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&DXGIFactory)); // windows 7 or above
	if (FAILED(hr))
	{
		OnD3DError(hr, LastResult);
		return;
	}
	LastResult = RenderAPI::eResult::Success;
}

uint64_t GetDriverVersion(const LUID& luid)
{
	static const char* const BaseKey = "SOFTWARE\\Microsoft\\DirectX";
	HKEY key;
	if (FAILED(RegOpenKeyExA(HKEY_LOCAL_MACHINE, BaseKey, 0, KEY_READ, &key)))
	{
		fprintf(stderr, "RegOpenKeyExA failed. Cannot figure out the driver version.\n");
		return 0;
	}
	DWORD length = MAX_PATH;
	DWORD numSubkeys = 0;
	DWORD maxSubkeyLen;
	DWORD maxClassLen;
	DWORD numValues;
	DWORD maxValueNameLen;
	DWORD maxValueLen;
	DWORD securityDescriptor;
	FILETIME lastWriteTime;
	if (FAILED(RegQueryInfoKeyA(key, nullptr, 0, 0, &numSubkeys, &maxSubkeyLen,
		&maxClassLen, &numValues, &maxValueNameLen, &maxValueLen, &securityDescriptor, &lastWriteTime)))
	{
		fprintf(stderr, "RegQueryInfoKeyA failed. Cannot figure out the driver version.\n");
		return 0;
	}

	for (DWORD i = 0; i < numSubkeys; ++i)
	{
		char strName[MAX_PATH] = {0};
		length = MAX_PATH;
		auto retCode = RegEnumKeyExA(key, i, strName, &length, nullptr, nullptr, 0, &lastWriteTime);
		if (retCode == ERROR_SUCCESS)
		{
			std::stringstream ss;
			ss << BaseKey << "\\" << strName;
			HKEY gpuKey;
			if (SUCCEEDED(RegOpenKeyExA(HKEY_LOCAL_MACHINE, ss.str().c_str(), 0, KEY_READ, &gpuKey)))
			{
				if (SUCCEEDED(RegQueryInfoKeyA(gpuKey, nullptr, 0, 0, nullptr, nullptr,
					nullptr, &numValues, &maxValueNameLen, &maxValueLen, nullptr, nullptr)))
				{
					bool found = false;
					for (DWORD vi = 0; vi < numValues; ++vi)
					{
						char valueName[MAX_PATH] = { 0 };
						DWORD valueNameLen = MAX_PATH;
						BYTE data[512];
						DWORD dataLen = 512;
						retCode = RegEnumValueA(gpuKey, vi, valueName, &valueNameLen, nullptr, nullptr, data, &dataLen);
						if (retCode == ERROR_SUCCESS) {
							if (_strcmpi("AdapterLuid", valueName) == 0)
							{
								LUID regLUID = *(LUID*)data;
								if (memcmp(data, &luid, sizeof(LUID)) == 0)
								{
									found = true;
								}
								break;
							}
						}
					
					}
					if (found)
					{
						for (DWORD vi = 0; vi < numValues; ++vi)
						{
							char valueName[MAX_PATH] = { 0 };
							DWORD valueNameLen = MAX_PATH;
							BYTE data[512];
							DWORD dataLen = 512;
							retCode = RegEnumValueA(gpuKey, vi, valueName, &valueNameLen, nullptr, nullptr, data, &dataLen);
							if (retCode == ERROR_SUCCESS) {
								if (_strcmpi("DriverVersion", valueName) == 0)
								{
									return *(uint64_t*)data;
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

inline std::string WideToAnsiString(wchar_t* source)
{
	const uint32_t BufferSize = 4096;
	std::array<char, BufferSize> ansiBuffer;
	memset(ansiBuffer.data(), 0, BufferSize);	
	int ret = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)source,
		-1, ansiBuffer.data(), BufferSize, NULL, NULL);

	if (ret == 0)
		std::cerr << "WideToAnsi Failed!";

	return std::string(ansiBuffer.data());
}


std::vector<PhysicalDeviceProperties> MPGEDirect3D12::GetGPUs() const
{
	LastResult = RenderAPI::eResult::Success;
	if (!DXGIFactory)
	{
		LastResult = RenderAPI::eResult::NotInitializedError;
		fprintf(stderr, "DXGI Factory is not initialized.\n");
		return {};
	}
	std::vector<PhysicalDeviceProperties> ret;

	UINT i = 0;
	IDXGIAdapter1* pAdapter;
	std::vector <IDXGIAdapter1*> vAdapters;
	while (DXGIFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		vAdapters.push_back(pAdapter);
		++i;
	}

	for (auto it : vAdapters)
	{
		DXGI_ADAPTER_DESC1 desc;
		auto hr = it->GetDesc1(&desc);
		if (FAILED(hr))
		{
			OnD3DError(hr, LastResult);
			return ret;
		}
		PhysicalDeviceProperties properties;
		properties.Name = WideToAnsiString(desc.Description);
		properties.DeviceId = desc.DeviceId;
		properties.DriverVersion = GetDriverVersion(desc.AdapterLuid);		
		properties.VendorId = desc.VendorId;
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

		properties.DedicatedVideoMemory = desc.DedicatedVideoMemory;
		ret.push_back(properties);
	}
	return ret;
}