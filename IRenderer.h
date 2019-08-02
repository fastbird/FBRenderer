#pragma once

namespace fb
{
	class IRenderer
	{
	public:
		virtual bool Initialize(void* windowHandle) = 0;
		virtual bool Finalize() = 0;

		virtual void OnResized() = 0;

		virtual void Draw(float dt) = 0;
	};
}
