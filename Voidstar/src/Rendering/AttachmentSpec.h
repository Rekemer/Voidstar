#pragma once
#include"Image.h"
namespace Voidstar
{

	struct AttachmentSpec
	{
		ImageSpecs Specs;
		// desired 
		size_t Amount = 3;
		vk::SampleCountFlagBits Samples;
	};
	struct SwapchainSpec : AttachmentSpec
	{
		vk::PresentModeKHR PresentMode;
		vk::ColorSpaceKHR ColorSpace;
	};
	struct DepthStencilSpecs : AttachmentSpec
	{
		// from most desirable to least desirable
		std::vector<vk::Format> Candidates;
		vk::FormatFeatureFlags  FormatFeature;
	};
}