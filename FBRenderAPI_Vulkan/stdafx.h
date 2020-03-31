#pragma once
#if defined(_MSC_VER)
#define MPGE_VULKAN_DLL __declspec(dllexport)
#endif

#pragma comment(lib, "vulkan-1.lib")
#include <vulkan/vulkan.hpp>