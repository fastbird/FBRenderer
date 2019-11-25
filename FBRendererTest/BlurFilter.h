#pragma once

#include "../IRenderer.h"

class BlurFilter
{
public:
	///<summary>
	/// The width and height should match the dimensions of the input texture to blur.
	/// Recreate when the screen is resized. 
	///</summary>
	BlurFilter(UINT width, UINT height, fb::EDataFormat format);

	BlurFilter(const BlurFilter& rhs) = delete;
	BlurFilter& operator=(const BlurFilter& rhs) = delete;
	~BlurFilter() = default;

	fb::ITextureIPtr Output();

	void BuildDescriptors(
		fb::IDescriptorHeapIPtr descriptorHeap, UINT offset);

	void OnResize(UINT newWidth, UINT newHeight);

	///<summary>
	/// Blurs the input texture blurCount times.
	///</summary>
	void Execute(
		const fb::IRootSignatureIPtr& rootSig,
		fb::PSOID horzBlurPSO,
		fb::PSOID vertBlurPSO,
		const fb::ITextureIPtr& input,
		int blurCount);

private:
	std::vector<float> CalcGaussWeights(float sigma);

	void BuildResources();

private:

	const int MaxBlurRadius = 5;

	UINT mWidth = 0;
	UINT mHeight = 0;
	fb::EDataFormat mFormat = fb::EDataFormat::R8G8B8A8_UNORM;

	fb::IDescriptorHeapIPtr DescriptorHeap;

	// Two for ping-ponging the textures.
	fb::ITextureIPtr mBlurMap0;
	fb::ITextureIPtr mBlurMap1;
};
