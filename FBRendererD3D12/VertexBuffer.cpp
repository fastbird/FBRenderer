#include "pch.h"
#include "VertexBuffer.h"
#include "RendererD3D12.h"
#include "UploadBuffer.h"
using namespace fb;

bool VertexBuffer::Initialize(const void* vertexData, UINT size, UINT stride, bool keepData)
{
	Stride = stride;
	if (stride != 0)
		NumVertices = size / stride;
	return GPUBuffer::Initialize(vertexData, size, keepData);
}

void VertexBuffer::Bind(int startSlot)
{
	gRendererD3D12->GetGraphicsCommandList()->IASetVertexBuffers(startSlot, 1, &VertexBufferView());
}

void VertexBuffer::FromUploadBuffer(IUploadBufferIPtr iUploadBuffer)
{
	if (!iUploadBuffer)
		return;
	auto uploadBuffer = ((UploadBuffer*)iUploadBuffer.get());
	Resource = uploadBuffer->GetResource();
	Stride = uploadBuffer->GetElementSize();
	Size = Stride * uploadBuffer->GetCount();
	if (Stride != 0)
		NumVertices = Size / Stride;
}