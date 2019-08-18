#pragma once
namespace fb
{
	enum class EDescriptorHeapType {
		None,
		Default, // for CBV, UAV, SRV
		Sampler,
	};
}