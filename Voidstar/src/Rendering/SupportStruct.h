#pragma once
#include"../Prereq.h"
#include "vulkan/vulkan.hpp"
namespace Voidstar
{
	class Device;
	struct SwapChainSupportDetails
	{
		// we have
		vk::SurfaceCapabilitiesKHR AvailableCapabilities;
		std::vector<vk::PresentModeKHR> AvailablePresentModes;
		std::vector<vk::SurfaceFormatKHR> AvailableFormats;
		uint32_t ViewportWidth, ViewportHeight;
		// we want
		vk::ColorSpaceKHR ColorSpace;
		vk::Format Format;
		vk::PresentModeKHR PresentMode;
		vk::ImageUsageFlags Usage;
		size_t FrameAmount;
	};
}
