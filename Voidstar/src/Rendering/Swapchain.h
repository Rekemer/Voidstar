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

		vk::Format GetFormat() const { return m_SwapchainFormat; }
		vk::Format GetDepthFormat() const { return m_SwapchainFrames[0].depthFormat; }
		vk::Extent2D GetExtent() const { return m_SwapchainExtent; }
		size_t GetFrameAmount() const { return m_SwapchainFrames.size(); }
		vk::Framebuffer GetFrameBuffer(size_t frameIndex)  const
		{
			assert(frameIndex < (GetFrameAmount())); 
			return m_SwapchainFrames[frameIndex].framebuffer;

		}
		SwapChainFrame& GetFrame(size_t frameIndex) 
		{
			assert(frameIndex < (GetFrameAmount()));
			return m_SwapchainFrames[frameIndex];
		}
		 std::vector<SwapChainFrame>& GetFrames() 
		 {
			return m_SwapchainFrames;
		 }

		void CreateMSAAFrame();
		vk::ImageView GetMSAAImageView() const { return m_MsaaImageView; }
		void CleanUp();
		~Swapchain();
		
	private:
		friend class Renderer;
		vk::SwapchainKHR m_Swapchain{ VK_NULL_HANDLE };
		std::vector<SwapChainFrame> m_SwapchainFrames;
		vk::Format m_SwapchainFormat;
		vk::Extent2D m_SwapchainExtent;
	
		vk::Image m_MsaaImage;
		vk::DeviceMemory m_MsaaImageMemory;
		vk::ImageView m_MsaaImageView;

	};
}