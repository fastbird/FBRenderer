#pragma once
#include "../FBCommon/glm.h"
#include "Types.h"
#include "IVertexBuffer.h"
#include "IIndexBuffer.h"
#include "PrimitiveTopology.h"
namespace fb
{
	struct RenderItem
	{
		RenderItem()
			: World(1.0f)
			, NumFramesDirty(2)
			, PrimitiveTopology(EPrimitiveTopology::TRIANGLELIST)
			, IndexCount(0)
			, StartIndexLocation(0)
			, BaseVertexLocation(0)
		{}

		glm::mat4 World;
		int NumFramesDirty;
		UINT ConstantBufferIndex = -1;
		IVertexBufferIPtr VB;
		IIndexBufferIPtr IB;
		EPrimitiveTopology PrimitiveTopology;

		// Draw params
		UINT IndexCount;
		UINT StartIndexLocation;
		int BaseVertexLocation;
	};
}
