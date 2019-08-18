#pragma once
#include "IRenderer.h"
namespace fb
{
	IRenderer* InitRenderer(RendererType type, void* windowHandle);
	void FinalizeRenderer(IRenderer*& renderer);
}
