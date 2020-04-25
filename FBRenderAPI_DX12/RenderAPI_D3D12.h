#pragma once
#include "../FBRenderAPI/RenderAPI.h"

extern "C" {
	MPGE_D3D12_DLL fb::RenderAPI* Initialize(fb::RenderAPI::eResult* ret, fb::InitInfo* initInfo);
}

namespace fb
{
	class MPGEDirect3D12 : public RenderAPI
	{
		MPGEDirect3D12(InitInfo* initInfo);
		~MPGEDirect3D12();

		mutable RenderAPI::eResult LastResult = RenderAPI::eResult::Success;
		Microsoft::WRL::ComPtr<IDXGIFactory4> DXGIFactory;
		Microsoft::WRL::ComPtr<ID3D12Device> Device;

	public:
		static MPGEDirect3D12* Initialize(InitInfo* initInfo);
		void Finalize() override;

		void CreateInstance(InitInfo* initInfo);
		RenderAPI::eResult GetLastResult() const { return LastResult; }

		std::vector<PhysicalDeviceProperties> GetGPUs() const override;
		// public functions end

	private:


	};
}