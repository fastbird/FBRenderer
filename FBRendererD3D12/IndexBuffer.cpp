#include "pch.h"
#include "IndexBuffer.h"
#include "ConverterD3D12.h"
#include "RendererD3D12.h"

using namespace fb;

bool IndexBuffer::Initialize(const void* indexData, UINT size, EIndexBufferFormat format, bool keepData)
{
	Format = format;
	switch (format) {
	case EIndexBufferFormat::R16:
	{
		ElementCount = size / 2;
		assert((float)ElementCount == size / 2.0f);
		break;
	}
	case EIndexBufferFormat::R32:
	{
		ElementCount = size / 4;
		assert((float)ElementCount == size / 4.0f);
		break;
	}
	default:
	{
		assert(0 && "Only R16_UINT and R32_UINT are allowed.");
		return false;
	}
	}
	return GPUBuffer::Initialize(indexData, size, keepData);
}

EIndexBufferFormat IndexBuffer::GetFormat() const
{
	return Format;
}

void IndexBuffer::Bind()
{
	gRendererD3D12->GetGraphicsCommandList()->IASetIndexBuffer(&IndexBufferView());
}