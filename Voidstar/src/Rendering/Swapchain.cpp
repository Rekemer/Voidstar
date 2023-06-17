#include "Prereq.h"
#include"Swapchain.h"
#include"SupportStruct.h"
#include"Device.h"
#include"../Log.h"
#include"RenderContext.h"
#include"Image.h"
namespace Voidstar
{
	vk::SurfaceFormatKHR GetSurfaceFormat(vk::Format format, vk::ColorSpaceKHR colorSpace, SwapChainSupportDetails& support);

	vk::Extent2D GetSwapchainExtent(uint32_t width, uint32_t height, SwapChainSupportDetails& support);

	vk::PresentModeKHR GetPresentMode(vk::PresentModeKHR presentMode, SwapChainSupportDetails& support);

	Swapchain* Swapchain::Create(SwapChainSupportDetails& support)
	{
		auto device = support.devcie;
		auto surface = support.surface;

		


		vk::SurfaceFormatKHR format = GetSurfaceFormat(vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear, support);



		vk::PresentModeKHR presentMode = GetPresentMode(vk::PresentModeKHR::eMailbox, support);


		auto viewportWidth = support.viewportWidth;
		auto viewportHeight = support.viewportHeight;
		vk::Extent2D extent = GetSwapchainExtent(viewportWidth, viewportHeight, support);

		uint32_t imageCount = std::min(
			support.capabilities.maxImageCount,
			support.capabilities.minImageCount + 1
		);
		
		vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
			vk::SwapchainCreateFlagsKHR(), *surface, imageCount, format.format, format.colorSpace,
			extent, 1, vk::ImageUsageFlagBits::eColorAttachment
		);



		uint32_t queueFamilyIndices[] = { device->GetGraphicsIndex(), device->GetGraphicsIndex() };

		if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		}

		createInfo.preTransform = support.capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

		Swapchain* swapchain = new Swapchain();
		try 
		{
			swapchain->m_Swapchain = device->GetDevice().createSwapchainKHR(createInfo);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("failed to create swap chain!");
		}


		// images already exists at this point
		auto images = device->GetDevice().getSwapchainImagesKHR(swapchain->m_Swapchain);
		swapchain->m_SwapchainFrames.resize(images.size());
		for (size_t i = 0; i < images.size(); ++i) {

			/*
			* ImageViewCreateInfo( VULKAN_HPP_NAMESPACE::ImageViewCreateFlags flags_ = {},
						   VULKAN_HPP_NAMESPACE::Image                image_ = {},
						   VULKAN_HPP_NAMESPACE::ImageViewType    viewType_  = VULKAN_HPP_NAMESPACE::ImageViewType::e1D,
						   VULKAN_HPP_NAMESPACE::Format           format_    = VULKAN_HPP_NAMESPACE::Format::eUndefined,
						   VULKAN_HPP_NAMESPACE::ComponentMapping components_            = {},
						   VULKAN_HPP_NAMESPACE::ImageSubresourceRange subresourceRange_ = {} ) VULKAN_HPP_NOEXCEPT
				: flags( flags_ )
				, image( image_ )
				, viewType( viewType_ )
				, format( format_ )
				, components( components_ )
				, subresourceRange( subresourceRange_ )
			*/

		

			swapchain->m_SwapchainFrames[i].image = images[i];
			swapchain->m_SwapchainFrames[i].imageView = Image::CreateImageView(images[i], format.format, vk::ImageAspectFlagBits::eColor);

			auto depthFormat = Image::GetFormat(
				device->GetDevicePhys(),
				{ vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint },
				vk::ImageTiling::eOptimal,
				vk::FormatFeatureFlagBits::eDepthStencilAttachment
			);

			ImageSpecs imageInfo;
			
			imageInfo.tiling = vk::ImageTiling::eOptimal;
			imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
			imageInfo.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
			imageInfo.width = viewportWidth;
			imageInfo.height = viewportHeight;
			imageInfo.format = depthFormat;


			auto samples = RenderContext::GetDevice()->GetSamples();
			swapchain->m_SwapchainFrames[i].imageDepth= Image::CreateVKImage(imageInfo, samples);
			swapchain->m_SwapchainFrames[i].depthImageMemory = Image::CreateMemory(swapchain->m_SwapchainFrames[i].imageDepth,imageInfo);
			swapchain->m_SwapchainFrames[i].imageDepthView = Image::CreateImageView(
				swapchain->m_SwapchainFrames[i].imageDepth, depthFormat, vk::ImageAspectFlagBits::eDepth
			);

			swapchain->m_SwapchainFrames[i].depthFormat = depthFormat;

		}

		swapchain->m_SwapchainFormat = format.format;
		swapchain->m_SwapchainExtent = extent;
		return swapchain;
		}

		Swapchain::~Swapchain()
		{
			auto device = RenderContext::GetDevice()->GetDevice();
			for (auto& frame : m_SwapchainFrames) {

				device.destroyImageView(frame.imageView);
				device.destroyFramebuffer(frame.framebuffer);
				device.freeMemory(frame.depthImageMemory);
				device.destroyImage(frame.imageDepth);
				device.destroyImageView(frame.imageDepthView);

			}
			// cannot not use detroy image on  presentable image
			device.destroySwapchainKHR(m_Swapchain);
		}
		



	vk::SurfaceFormatKHR GetSurfaceFormat(vk::Format format, vk::ColorSpaceKHR colorSpace, SwapChainSupportDetails& support)
	{
		for (vk::SurfaceFormatKHR formatSupport : support.formats)
		{
			if (formatSupport.format == format
				&& formatSupport.colorSpace == colorSpace) {
				return formatSupport;
			}
		}

		return support.formats[0];
	}

	vk::PresentModeKHR GetPresentMode(vk::PresentModeKHR presentMode, SwapChainSupportDetails& support)
	{
		for (vk::PresentModeKHR presentModeSupport : support.presentModes)
		{
			if (presentMode == vk::PresentModeKHR::eMailbox) {
				presentMode = presentModeSupport;
			}
		}

		return vk::PresentModeKHR::eFifo;
	}
	vk::Extent2D GetSwapchainExtent(uint32_t width, uint32_t height, SwapChainSupportDetails& support) {

		auto capabilities = support.capabilities;
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			vk::Extent2D extent = { width, height };

			extent.width = std::min(
				capabilities.maxImageExtent.width,
				std::max(capabilities.minImageExtent.width, extent.width)
			);

			extent.height = std::min(
				capabilities.maxImageExtent.height,
				std::max(capabilities.minImageExtent.height, extent.height)
			);

			return extent;
		}
	}
}