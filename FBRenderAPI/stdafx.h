#pragma once
#if defined(_MSC_VER)
#define MPGE_DLL __declspec(dllexport)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
//#pragma comment(lib, "Kernel32.lib")
#endif

#include <iostream>