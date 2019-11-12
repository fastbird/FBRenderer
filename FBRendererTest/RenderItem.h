#pragma once
#include "../../FBCommon/glm.h"
#include "../IVertexBuffer.h"
#include "../IIndexBuffer.h"
#include "../ITexture.h"
#include "../PrimitiveTopology.h"
#include "Material.h"


struct RenderItem
{
	RenderItem()
		: World(1.0f)
		, TexTransform(1.0f)
		, NumFramesDirty(NUM_SWAPCHAIN_BUFFERS)
		, PrimitiveTopology(fb::EPrimitiveTopology::TRIANGLELIST)
		, IndexCount(0)
		, StartIndexLocation(0)
		, BaseVertexLocation(0)
	{}

	glm::mat4 World;
	glm::mat4 TexTransform;
	int NumFramesDirty;
	UINT ObjectCBIndex = -1;
	Material* Mat = nullptr;
	fb::ITextureIPtr Texture;
	fb::IVertexBufferIPtr VB;
	fb::IIndexBufferIPtr IB;
	fb::EPrimitiveTopology PrimitiveTopology;

	// Draw params
	UINT IndexCount;
	UINT StartIndexLocation;
	int BaseVertexLocation;
};
