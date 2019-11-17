#pragma once
#include "GPUBuffer.h"
#include "../IRenderer.h"
#include "ConverterD3D12.h"
namespace fb
{
	class IndexBuffer : public GPUBuffer, public IIndexBuffer
	{
		EIndexBufferFormat Format;
		UINT ElementCount;

	public:
		virtual bool Initialize(const void* indexData, UINT size, EIndexBufferFormat format, bool keepData) override;
		virtual UINT GetSize() const override { return Size; }
		virtual EIndexBufferFormat GetFormat() const override;
		virtual UINT GetElementCount() const override { return ElementCount; }
		D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
		{
			return D3D12_INDEX_BUFFER_VIEW{ Resource->GetGPUVirtualAddress(), Size, Convert(Format) };
		}
		virtual void Bind() override;
	};
}