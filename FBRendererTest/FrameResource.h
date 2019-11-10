#pragma once
#include "../ICommandAllocator.h"
#include "../IUploadBuffer.h"
#include "../Types.h"
#include "Light.h"

struct ObjectConstants
{
	glm::mat4 World = glm::mat4(1.0f);
	glm::mat4 TexTransform = glm::mat4(1.0f);
};

struct MaterialConstants
{
	glm::vec4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = .25f;
	glm::mat4 MatTransform;
};

struct PassConstants
{
	glm::mat4 View = glm::mat4(1.0f);
	glm::mat4 InvView = glm::mat4(1.0f);
	glm::mat4 Proj = glm::mat4(1.0f);
	glm::mat4 InvProj = glm::mat4(1.0f);
	glm::mat4 ViewProj = glm::mat4(1.0f);
	glm::mat4 InvViewProj = glm::mat4(1.0f);
	glm::vec3 EyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerObjectPad1 = 0.0f;
	glm::vec2 RenderTargetSize = { 0.0f, 0.0f };
	glm::vec2 InvRenderTargetSize = { 0.0f, 0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;

	glm::vec4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
	Light Lights[MaxLights];
};

struct FFrameResource
{
	fb::ICommandAllocatorIPtr CommandAllocator;
	fb::IUploadBufferIPtr CBPerObject;
	fb::IUploadBufferIPtr CBPerMaterial;
	fb::IUploadBufferIPtr CBPerFrame;
	fb::IUploadBufferIPtr WavesVB;
	fb::UINT64 Fence = 0;
};

extern std::vector<FFrameResource> FrameResources;

void BuildFrameResources();
void DestroyFrameResources();
void BuildConstantBuffers(int numObj);
inline FFrameResource& GetFrameResource(UINT index) { return FrameResources[index]; }
FFrameResource& GetFrameResource_WaitAvailable(UINT index);