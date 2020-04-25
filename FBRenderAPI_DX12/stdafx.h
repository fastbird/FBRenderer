#pragma once
#if defined(_MSC_VER)
#define MPGE_D3D12_DLL __declspec(dllexport)
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winreg.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <comdef.h>

#include <sstream>
#include <array>