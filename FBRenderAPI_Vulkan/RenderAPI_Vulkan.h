#pragma once
#include "../FBRenderAPI/RenderAPI.h"

extern "C" {
	MPGE_VULKAN_DLL fb::RenderAPI* Initialize(fb::RenderAPI::Result* ret, fb::InitInfo* initInfo);
}

namespace fb
{
	class MPGEVulkan : public RenderAPI
	{
		MPGEVulkan(InitInfo* initInfo);
		~MPGEVulkan();

		RenderAPI::Result LastResult = RenderAPI::Result::Success;

	public:
		static MPGEVulkan* Initialize(InitInfo* initInfo);
		void Finalize() override;

		void CreateInstance(InitInfo* initInfo);
		RenderAPI::Result GetLastResult() const { return LastResult; }

		// public functions end

	private:
		bool IsSupportedLayer(const char* layerName) const;

		
		
	};
}