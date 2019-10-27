#pragma once
struct Material
{
	Material()
		:MatTransform(1.0f)
	{

	}
	std::string Name;
	int MatCBIndex = -1;
	int DiffuseSrvHeapIndex = -1;
	int NormalSrvHeapIndex = -1;
	int NumFramesDirty = NUM_SWAPCHAIN_BUFFERS;

	glm::vec4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = .25f;
	glm::mat4 MatTransform;
};
