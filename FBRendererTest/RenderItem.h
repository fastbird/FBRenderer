#pragma once
#include "../../FBCommon/glm.h"
#include "../IVertexBuffer.h"
#include "../IIndexBuffer.h"
#include "../PrimitiveTopology.h"
#include "Material.h"

struct RenderItem
{
	RenderItem()
		: World(1.0f)
		, NumFramesDirty(NUM_SWAPCHAIN_BUFFERS)
		, PrimitiveTopology(fb::EPrimitiveTopology::TRIANGLELIST)
		, IndexCount(0)
		, StartIndexLocation(0)
		, BaseVertexLocation(0)
	{}

	glm::mat4 World;
	int NumFramesDirty;
	UINT ObjectCBIndex = -1;
	Material* Mat = nullptr;
	fb::IVertexBufferIPtr VB;
	fb::IIndexBufferIPtr IB;
	fb::EPrimitiveTopology PrimitiveTopology;

	// Draw params
	UINT IndexCount;
	UINT StartIndexLocation;
	int BaseVertexLocation;
};
