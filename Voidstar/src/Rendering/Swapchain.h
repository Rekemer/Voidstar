#pragma once
#include"../Prereq.h"
#include "vulkan/vulkan.hpp"
#include "Image.h"
namespace Voidstar
{
	/*struct SwapChainFrame {
		SwapchainImage ColourImage;
		SwapchainImage DepthImage;
		
		vk::Framebuffer framebuffer;


	};*/

	struct SwapChainSupportDetails;
	class Device;
	class Swapchain
	{
	public:
		static UPtr<Swapchain> Create(SwapChainSupportDetails& info);

		vk::Format GetFormat() const { return m_SwapchainFormat; }
		//vk::Format GetDepthFormat()  { return m_SwapchainFrames[0].DepthImage.GetFormat(); }
		vk::Extent2D GetExtent() const { return m_SwapchainExtent; }
		size_t GetFrameAmount() const { return m_Images.size(); }
		std::vector<std::shared_ptr<SwapchainImage>> GetImages()  { return m_Images; }
		//vk::Framebuffer GetFrameBuffer(size_t frameIndex)  const
		//{
		//	assert(frameIndex < (GetFrameAmount())); 
		//	return m_SwapchainFrames[frameIndex].framebuffer;
		//
		//}
		/*SwapChainFrame& GetFrame(size_t frameIndex) 
		{
			assert(frameIndex < (GetFrameAmount()));
			return m_SwapchainFrames[frameIndex];
		}
		 std::vector<SwapChainFrame>& GetFrames() 
		 {
			return m_SwapchainFrames;
		 }*/

		void CreateMSAAFrame();
	//	vk::ImageView GetMSAAImageView() const { return m_MsaaImageView; }
		void CleanUp();
		~Swapchain();
		
	private:
		friend class Renderer;
		vk::SwapchainKHR m_Swapchain{ VK_NULL_HANDLE };
		vk::Format m_SwapchainFormat;
		vk::Extent2D m_SwapchainExtent;
		std::vector<std::shared_ptr<SwapchainImage>> m_Images;
	//	vk::Image m_MsaaImage;
	//	vk::DeviceMemory m_MsaaImageMemory;
	//	vk::ImageView m_MsaaImageView;

	};
}