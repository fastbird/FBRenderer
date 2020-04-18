#include "pch.h"
#include "BlurFilter.h"

extern fb::IRenderer* gRenderer;
BlurFilter::BlurFilter(UINT width, UINT height, fb::EDataFormat format)
{
	mWidth = width;
	mHeight = height;
	mFormat = format;

	BuildResources();
}

fb::ITextureIPtr BlurFilter::Output()
{
	return mBlurMap0;
}

void BlurFilter::BuildDescriptors(fb::IDescriptorHeapIPtr descriptorHeap, UINT offset)
{
	DescriptorHeap = descriptorHeap;
	DescriptorHeap->CreateSRV(offset, mBlurMap0);
	DescriptorHeap->CreateUAV(offset + 1, mBlurMap0);
	DescriptorHeap->CreateSRV(offset + 2, mBlurMap1);
	DescriptorHeap->CreateUAV(offset + 3, mBlurMap1);
}

void BlurFilter::OnResize(UINT newWidth, UINT newHeight)
{
	if ((mWidth != newWidth) || (mHeight != newHeight))
	{
		mWidth = newWidth;
		mHeight = newHeight;

		BuildResources();

		// New resource, so we need new descriptors to that resource.
		//BuildDescriptors();
	}
}

void BlurFilter::Execute(const fb::IRootSignatureIPtr& rootSig,
	fb::PSOID horzBlurPSO,
	fb::PSOID vertBlurPSO,
	const fb::ITextureIPtr& input,
	int blurCount)
{
	auto weights = CalcGaussWeights(2.5f);
	int blurRadius = (int)weights.size() / 2;
	rootSig->Bind();
	gRenderer->SetComputeRoot32BitConstants(0, 1, &blurRadius, 0);
	gRenderer->SetComputeRoot32BitConstants(0, (UINT)weights.size(), weights.data(), 1);

	gRenderer->ResourceBarrierTransition(input, fb::FResourceBarrier{ fb::EResourceState::RENDER_TARGET, fb::EResourceState::COPY_SOURCE });
	gRenderer->ResourceBarrierTransition(mBlurMap0, fb::FResourceBarrier{ fb::EResourceState::COMMON, fb::EResourceState::COPY_DEST });
	// Copy the input (back-buffer in this example) to BlurMap0.
	gRenderer->CopyResource(mBlurMap0, input);

	gRenderer->ResourceBarrierTransition(mBlurMap0, fb::FResourceBarrier{ fb::EResourceState::COPY_DEST, fb::EResourceState::STATE_GENERIC_READ });
	gRenderer->ResourceBarrierTransition(mBlurMap1, fb::FResourceBarrier{ fb::EResourceState::COMMON, fb::EResourceState::UNORDERED_ACCESS });

	for (int i = 0; i < blurCount; ++i)
	{
		//
		// Horizontal Blur pass.
		//

		cmdList->SetPipelineState(horzBlurPSO);

		cmdList->SetComputeRootDescriptorTable(1, mBlur0GpuSrv);
		cmdList->SetComputeRootDescriptorTable(2, mBlur1GpuUav);

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).
		UINT numGroupsX = (UINT)ceilf(mWidth / 256.0f);
		cmdList->Dispatch(numGroupsX, mHeight, 1);

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap0.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap1.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));

		//
		// Vertical Blur pass.
		//

		cmdList->SetPipelineState(vertBlurPSO);

		cmdList->SetComputeRootDescriptorTable(1, mBlur1GpuSrv);
		cmdList->SetComputeRootDescriptorTable(2, mBlur0GpuUav);

		// How many groups do we need to dispatch to cover a column of pixels, where each
		// group covers 256 pixels  (the 256 is defined in the ComputeShader).
		UINT numGroupsY = (UINT)ceilf(mHeight / 256.0f);
		cmdList->Dispatch(mWidth, numGroupsY, 1);

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap0.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap1.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	}
}

std::vector<float> BlurFilter::CalcGaussWeights(float sigma)
{
	float twoSigma2 = 2.0f * sigma * sigma;

	// Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
	// For example, for sigma = 3, the width of the bell curve is 
	int blurRadius = (int)ceil(2.0f * sigma);

	assert(blurRadius <= MaxBlurRadius);

	std::vector<float> weights;
	weights.resize(2 * blurRadius + 1);

	float weightSum = 0.0f;

	for (int i = -blurRadius; i <= blurRadius; ++i)
	{
		float x = (float)i;

		weights[i + blurRadius] = expf(-x * x / twoSigma2);

		weightSum += weights[i + blurRadius];
	}

	// Divide by the sum so all the weights add up to 1.0.
	for (int i = 0; i < weights.size(); ++i)
	{
		weights[i] /= weightSum;
	}

	return weights;
}

void BlurFilter::BuildResources()
{
	// Note, compressed formats cannot be used for UAV.  We get error like:
	// ERROR: ID3D11Device::CreateTexture2D: The format (0x4d, BC3_UNORM) 
	// cannot be bound as an UnorderedAccessView, or cast to a format that
	// could be bound as an UnorderedAccessView.  Therefore this format 
	// does not support D3D11_BIND_UNORDERED_ACCESS.

	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mWidth;
	texDesc.Height = mHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = mFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mBlurMap0)));

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mBlurMap1)));
}