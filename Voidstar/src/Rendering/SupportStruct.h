#pragma once
#include"../Prereq.h"
#include "vulkan/vulkan.hpp"
namespace Voidstar
{
	class Device;
	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
		uint32_t viewportWidth, viewportHeight;
		vk::SurfaceKHR* surface;
		Device* devcie;
	};
}
