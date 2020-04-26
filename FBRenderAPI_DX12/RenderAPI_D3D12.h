#pragma once
#include "../FBRenderAPI/RenderAPI.h"

extern "C" {
	MPGE_D3D12_DLL fb::RenderAPI* Initialize(fb::RenderAPI::eResult* ret, fb::InitInfo* initInfo);
}

namespace fb
{
	class DeviceD3D12 : public RenderAPI::Device
	{
	public:
		DeviceD3D12(Microsoft::WRL::ComPtr<ID3D12Device> device);


		Microsoft::WRL::ComPtr<ID3D12Device> Device;
	};

	class MPGEDirect3D12 : public RenderAPI
	{
		MPGEDirect3D12(InitInfo* initInfo);
		~MPGEDirect3D12();

		mutable RenderAPI::eResult LastResult = RenderAPI::eResult::Success;
		Microsoft::WRL::ComPtr<IDXGIFactory4> DXGIFactory;
		DeviceD3D12* Device = nullptr;
		

	public:
		static MPGEDirect3D12* Initialize(InitInfo* initInfo);
		void Finalize() override;

		void CreateInstance(InitInfo* initInfo);
		RenderAPI::eResult GetLastResult() const { return LastResult; }

		std::vector<PhysicalDeviceProperties> GetGPUs() const override;
		virtual RenderAPI::Device* CreateDevice(uint32_t gpuIndex) override;
		// public functions end

	private:


	};
}