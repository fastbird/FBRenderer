#pragma once
#include <vector>
#include <functional>
#include "Types.h"
#include "../FBCommon/IRefCounted.h"
#include "DataFormat.h"
#include "InputElementDesc.h"
#include "IShader.h"
#include "PSO.h"
#include "IUploadBuffer.h"
#include "ICommandAllocator.h"
#include "EDescriptorHeapType.h"
#include "IVertexBuffer.h"
#include "IIndexBuffer.h"
#include "IRootSignature.h"

namespace fb
{
	enum class RendererType {
		D3D12
	};	

	using DrawCallbackFunc = void (*)();
	class IRenderer
	{
		friend IRenderer* InitRenderer(RendererType type, void* windowHandle);
		friend void FinalizeRenderer(IRenderer*& renderer);
		virtual bool Initialize(void* windowHandle) = 0;
		virtual void Finalize() = 0;

	public:	

		virtual void PrepareDescriptorHeap(EDescriptorHeapType heapType, UINT count) = 0;
		virtual void WaitFence(UINT64 fence) = 0;

		virtual int GetNumSwapchainBuffers() = 0;
		virtual void OnResized() = 0;

		virtual ICommandAllocator* CreateCommandAllocator() = 0;
		virtual IVertexBuffer* CreateVertexBuffer(const void* vertexData, UINT size, UINT stride, bool keepData) = 0;
		virtual IIndexBuffer* CreateIndexBuffer(const void* indexData, UINT size, EDataFormat format, bool keepData) = 0;
		virtual IUploadBuffer* CreateUploadBuffer(UINT elementSize, UINT count, bool constantBuffer) = 0;
		virtual PSOID CreateGraphicsPipelineState(const FPSODesc& psoDesc) = 0;
		virtual void DestroyGraphicsPipelineState(PSOID psoid) = 0;
		virtual IShader* CompileShader(const char* filepath, FShaderMacro* macros, int numMacros, EShaderType shaderType, const char* entryFunctionName) = 0;
		virtual EDataFormat GetBackBufferFormat() const = 0;
		virtual EDataFormat GetDepthStencilFormat() const = 0;
		// definition format : type,num,gpuIndex; i.e. DTable,1,0
		// types are defined in IRootSignature.h
		virtual IRootSignature* CreateRootSignature(const char* definition) = 0;
		virtual int GetSampleCount() const = 0;
		virtual int GetMsaaQuality() const = 0;
		virtual int GetBackbufferWidth() const = 0;
		virtual int GetBackbufferHeight() const = 0;
		virtual void ResetCommandList(ICommandAllocatorIPtr cmdAllocator, PSOID pso) = 0;
		virtual void CloseCommandList() = 0;
		virtual void ExecuteCommandList() = 0;
		virtual void PresentAndSwapBuffer() = 0;
		virtual void FlushCommandQueue() = 0;
		virtual void BindDescriptorHeap(EDescriptorHeapType type) = 0;
		virtual void SetGraphicsRootConstantBufferView(int rootParamIndex, fb::IUploadBufferIPtr constantBuffer, int offset) = 0;
		virtual void SetGraphicsRootDescriptorTable(int rootParamIndex, fb::EDescriptorHeapType heapType, int index) = 0;
		virtual void SetPrimitiveTopology(const fb::EPrimitiveTopology topology) = 0;
		virtual void DrawIndexedInstanced(UINT IndexCountPerInstance,
			UINT InstanceCount,
			UINT StartIndexLocation,
			INT BaseVertexLocation,
			UINT StartInstanceLocation) = 0;
		virtual void ResourceBarrier_Backbuffer_PresentToRenderTarget() = 0;
		virtual void ResourceBarrier_Backbuffer_RenderTargetToPresent() = 0;
		virtual void SetViewportAndScissor(UINT width, UINT height) = 0;
		virtual void ClearRenderTargetDepthStencil() = 0;
		virtual void SetDefaultRenderTargets() = 0;		
		virtual UINT64 SignalFence() = 0;
		

		virtual void TempResetCommandList() = 0;		
		virtual void TempCreateRootSignatureForSimpleBox() = 0;
		virtual void TempBindVertexBuffer(const IVertexBufferIPtr& vb) = 0;
		virtual void TempBindIndexBuffer(const IIndexBufferIPtr& ib) = 0;		
		virtual void TempBindRootDescriptorTable(UINT slot, EDescriptorHeapType type) = 0;
		virtual void TempDrawIndexedInstanced(UINT indexCount) = 0;
		

		UINT CalcConstantBufferByteSize(UINT beforeAligned) const;
	};
}
