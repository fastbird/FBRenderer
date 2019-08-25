#pragma once
#include <vector>
#include "Types.h"
#include "../FBCommon/IRefCounted.h"
#include "DataFormat.h"
#include "InputElementDesc.h"
#include "IShader.h"
#include "PSO.h"
#include "FrameResource.h"
#include "EDescriptorHeapType.h"
#include "IVertexBuffer.h"
#include "IIndexBuffer.h"
#include <functional>
namespace fb
{
	enum class RendererType {
		D3D12
	};	

	using DrawCallbackFunc = void (*)();
	class IRenderer
	{
		std::vector<FFrameResource> FrameResources;

		friend IRenderer* InitRenderer(RendererType type, void* windowHandle);
		friend void FinalizeRenderer(IRenderer*& renderer);
		virtual bool Initialize(void* windowHandle) = 0;
		virtual void Finalize() = 0;
		void BuildFrameResources();

	public:
		
		
		FFrameResource& GetFrameResource(UINT index) { return FrameResources[index]; }
		FFrameResource& GetFrameResource_WaitAvailable(UINT index);

		virtual void PrepareDescriptorHeap(EDescriptorHeapType heapType, UINT count) = 0;
		virtual void WaitFence(UINT64 fence) = 0;

		virtual int GetNumSwapchainBuffers() = 0;
		virtual void OnResized() = 0;
		virtual void RegisterDrawCallback(DrawCallbackFunc func) = 0;
		virtual void Draw(float dt) = 0;

		virtual ICommandAllocator* CreateCommandAllocator() = 0;
		virtual IVertexBuffer* CreateVertexBuffer(const void* vertexData, UINT size, UINT stride, bool keepData) = 0;
		virtual IIndexBuffer* CreateIndexBuffer(const void* indexData, UINT size, EDataFormat format, bool keepData) = 0;
		virtual IUploadBuffer* CreateUploadBuffer(UINT elementSize, UINT count, bool constantBuffer, EDescriptorHeapType heapType) = 0;
		virtual PSOID CreateGraphicsPipelineState(const FPSODesc& psoDesc) = 0;
		virtual IShader* CompileShader(const char* filepath, FShaderMacro* macros, int numMacros, EShaderType shaderType, const char* entryFunctionName) = 0;
		virtual EDataFormat GetBackBufferFormat() const = 0;
		virtual EDataFormat GetDepthStencilFormat() const = 0;
		virtual int GetSampleCount() const = 0;
		virtual int GetMsaaQuality() const = 0;
		virtual int GetBackbufferWidth() const = 0;
		virtual int GetBackbufferHeight() const = 0;
		
		virtual void TempResetCommandList() = 0;
		virtual void TempCloseCommandList(bool runAndFlush) = 0;
		virtual void TempBindDescriptorHeap(EDescriptorHeapType type) = 0;
		virtual void TempCreateRootSignatureForSimpleBox() = 0;
		virtual RootSignature TempGetRootSignatureForSimpleBox() = 0;
		virtual void TempBindRootSignature(RootSignature rootSig) = 0;
		virtual void TempBindVertexBuffer(const IVertexBufferIPtr& vb) = 0;
		virtual void TempBindIndexBuffer(const IIndexBufferIPtr& ib) = 0;
		virtual void TempSetPrimitiveTopology(const fb::EPrimitiveTopology topology) = 0;
		virtual void TempBindRootDescriptorTable(UINT slot, EDescriptorHeapType type) = 0;
		virtual void TempDrawIndexedInstanced(UINT indexCount) = 0;

		UINT CalcConstantBufferByteSize(UINT beforeAligned) const;
	};
}
