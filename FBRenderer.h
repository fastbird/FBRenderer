#pragma once
#include "IRenderer.h"
namespace fb
{
	class IRenderer;
	enum class RendererType {
		D3D12
	};
	IRenderer* InitRenderer(RendererType type, void* windowHandle);
}
