#pragma once
#include"../Prereq.h"
#include "vulkan/vulkan.hpp"
#include "Image.h"
namespace Voidstar
{
	

	struct SwapChainSupportDetails;
	class Device;
	class VOIDSTAR_API Swapchain
	{
	public:
		static UPtr<Swapchain> Create(SwapChainSupportDetails& info);

		vk::Format GetFormat() const { return m_SwapchainFormat; }
		vk::Extent2D GetExtent() const { return m_SwapchainExtent; }
		size_t GetFrameAmount() const { return m_Images.size(); }
		std::vector<std::shared_ptr<Image>> GetImages()  { return m_Images; }
		void CleanUp();
		
	private:
		friend class Renderer;
		vk::SwapchainKHR m_Swapchain{ VK_NULL_HANDLE };
		vk::Format m_SwapchainFormat;
		vk::Extent2D m_SwapchainExtent;
		std::vector<std::shared_ptr<Image>> m_Images;
	};
}