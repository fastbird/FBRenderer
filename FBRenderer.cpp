#include "FBRenderer.h"
#include <exception>
#include <iostream>
#include "IRenderer.h"

// for windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace fb;

typedef fb::IRenderer* (*CreateRendererD3D12)();
IRenderer* fb::InitRenderer(RendererType type, void* windowHandle)
{
	std::cout << "fb::InitRenderer" << std::endl;
	switch (type)
	{
	case RendererType::D3D12:
	{
		HMODULE hmodule = LoadLibrary("FBRendererD3D12.dll");
		if (!hmodule)
		{
			std::cout << "Cannot load FBRendererD3D12.dll" << std::endl;
			return nullptr;
		}
		auto createFunc = (CreateRendererD3D12)GetProcAddress(hmodule, "CreateRendererD3D12");
		auto renderer = createFunc();
		if (!renderer)
		{
			std::cout << "Failed to create Renderer." << std::endl;
			return nullptr;
		}
		renderer->Initialize(windowHandle);
		return renderer;

	}
	}
	std::cout << "Failed to initialize renderer!" << std::endl;
	return nullptr;
}

void fb::FinalizeRenderer(IRenderer*& renderer)
{
	renderer->Finalize();
	renderer = nullptr;
}