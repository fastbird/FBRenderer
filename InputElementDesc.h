#pragma once
#include <string>
#include "Types.h"
#include "DataFormat.h"

namespace fb
{
	enum class EInputClassification
	{
		PerVertexData,
		PerInstanceData
	};

	enum class EVertexElementType : UINT
	{
		Position,
		Color,
		UV,
		Normal,
	};

	struct FInputElementDesc
	{
		EVertexElementType Type;
		UINT Index; // UV0? UV1?
		EDataFormat Format;
		UINT InputSlot; // index of the vertex buffer. In case we use multiple vertex buffers to form a complete vertices stream.
		UINT AlignedByteOffset;
		EInputClassification InputSlotClass;
		UINT InstanceDataStepRate;
	};

	struct FInputLayoutDesc
	{
		const FInputElementDesc* pInputElementDescs;
		UINT NumElements;
	};
}