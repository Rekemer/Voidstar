#pragma once
#include"../Prereq.h"
#include "vulkan/vulkan.hpp"
namespace Voidstar
{
	struct SwapChainFrame {
		vk::Image image;
		vk::ImageView imageView;

		VkDeviceMemory depthImageMemory;
		vk::Image imageDepth;
		vk::ImageView imageDepthView;
		vk::Format depthFormat;
		
		vk::Framebuffer framebuffer;
	};

	struct SwapChainSupportDetails;
	class Device;
	class Swapchain
	{
	public:
		static UPtr<Swapchain> Create(SwapChainSupportDetails& info);
		~Swapchain();
		std::vector<SwapChainFrame>& GetFrames()
		{
			return m_SwapchainFrames;
		}
		vk::Extent2D& GetExtent()
		{
			return m_SwapchainExtent;
		}
		vk::Format& GetFormat()
		{
			return m_SwapchainFormat;
		}
		vk::SwapchainKHR& GetSwapChain()
		{
			return m_Swapchain;
		}
		size_t GetFramesCount() { return m_SwapchainFrames.size(); }
	private:
		vk::SwapchainKHR m_Swapchain{ VK_NULL_HANDLE };
		std::vector<SwapChainFrame> m_SwapchainFrames;
		vk::Format m_SwapchainFormat;
		vk::Extent2D m_SwapchainExtent;
	
	};
}