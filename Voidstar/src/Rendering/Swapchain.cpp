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

	// format, extent, usage
	UPtr<Swapchain> Swapchain::Create(SwapChainSupportDetails& support)
	{
		auto device = RenderContext::GetDevice();
		auto surface = RenderContext::GetSurface();

		


		//vk::SurfaceFormatKHR format = GetSurfaceFormat(vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear, support);
		vk::SurfaceFormatKHR format = GetSurfaceFormat(support.Format, support.ColorSpace, support);



		//vk::PresentModeKHR presentMode = GetPresentMode(vk::PresentModeKHR::eFifo, support);
		vk::PresentModeKHR presentMode = GetPresentMode(support.PresentMode, support);


		auto viewportWidth = support.ViewportWidth;
		auto viewportHeight = support.ViewportHeight;
		vk::Extent2D extent = GetSwapchainExtent(viewportWidth, viewportHeight, support);

		uint32_t imageCount = std::min(
			support.AvailableCapabilities.maxImageCount,
			(uint32_t)support.FrameAmount
		);
		
		vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
			vk::SwapchainCreateFlagsKHR(), *surface, imageCount, format.format, format.colorSpace,
			extent, 1, support.Usage
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

		createInfo.preTransform = support.AvailableCapabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

		auto swapchain = CreateUPtr<Swapchain>();
		try 
		{
			swapchain->m_Swapchain = device->GetDevice().createSwapchainKHR(createInfo);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("failed to create swap chain!");
		}
		auto images = device->GetDevice().getSwapchainImagesKHR(swapchain->m_Swapchain);
		swapchain->m_Images.resize(images.size());
		for (int i =0; i < images.size(); i++)
		{
			swapchain->m_Images[i].m_Format= format.format;
			swapchain->m_Images[i].m_Image = images[i];
			swapchain->m_Images[i].m_ImageView = 
				Image::CreateImageView(images[i], format.format, vk::ImageAspectFlagBits::eColor);
		}
		return swapchain;
		//for (size_t i = 0; i < images.size(); ++i) {


		//	swapchain->m_SwapchainFrames[i].ColourImage.m_Image = images[i];


		//	// just for depth
		//	auto defCandidates = { vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint };
		//	// for stencil
		//	std::vector<vk::Format> candidates = 
		//	{
		//		vk::Format::eD32SfloatS8Uint,
		//		vk::Format::eD24UnormS8Uint,
		//		vk::Format::eD16UnormS8Uint

		//	};
		//	auto depthFormat = Image::GetFormat(
		//		device->GetDevicePhys(),
		//		candidates,
		//		vk::ImageTiling::eOptimal,
		//		vk::FormatFeatureFlagBits::eDepthStencilAttachment
		//	);

		//	ImageSpecs imageInfo;
		//	
		//	imageInfo.tiling = vk::ImageTiling::eOptimal;
		//	imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment| vk::ImageUsageFlagBits::eInputAttachment;
		//	imageInfo.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		//	imageInfo.width = viewportWidth;
		//	imageInfo.height = viewportHeight;
		//	imageInfo.format = depthFormat;


		//	auto samples = RenderContext::GetDevice()->GetSamples();
		//	swapchain->m_SwapchainFrames[i].imageDepth= Image::CreateVKImage(imageInfo, samples);
		//	swapchain->m_SwapchainFrames[i].depthImageMemory = Image::CreateMemory(swapchain->m_SwapchainFrames[i].imageDepth,imageInfo);
		//	swapchain->m_SwapchainFrames[i].imageDepthView = Image::CreateImageView(
		//	swapchain->m_SwapchainFrames[i].imageDepth, depthFormat, vk::ImageAspectFlagBits::eDepth
		//	);

		//	swapchain->m_SwapchainFrames[i].depthFormat = depthFormat;

		//}

		//swapchain->m_SwapchainFormat = format.format;
		//swapchain->m_SwapchainExtent = extent;
		
		}

		void Swapchain::CreateMSAAFrame()
		{
			ImageSpecs specs;
			auto extent = m_SwapchainExtent;
			auto swapchainFormat = m_SwapchainFormat;
			specs.width = extent.width;
			specs.height = extent.height;
			specs.tiling = vk::ImageTiling::eOptimal;
			specs.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment;
			specs.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
			specs.format = swapchainFormat;
			auto samples = RenderContext::GetDevice()->GetSamples();
			//m_MsaaImage = Image::CreateVKImage(specs, samples);
			//m_MsaaImageMemory = Image::CreateMemory(m_MsaaImage, specs);
			//m_MsaaImageView = Image::CreateImageView(m_MsaaImage, swapchainFormat, vk::ImageAspectFlagBits::eColor);
		}
		
		void Swapchain::CleanUp()
		{
			auto device = RenderContext::GetDevice()->GetDevice();
			//for (auto& e : m_Images)
			//{
			//	e.Destroy();
			//}
			//for (auto& frame : m_SwapchainFrames) {

			//	 // image is destroyed with swapchain
			//	device.destroyImageView(frame.imageView);
			//	device.destroyFramebuffer(frame.framebuffer);
			//	device.freeMemory(frame.depthImageMemory);
			//	device.destroyImage(frame.imageDepth);
			//	device.destroyImageView(frame.imageDepthView);

			//}

			//device.freeMemory(m_MsaaImageMemory);
			//device.destroyImage(m_MsaaImage);
			//device.destroyImageView(m_MsaaImageView);

			// cannot not use detroy image on  presentable image
			device.destroySwapchainKHR(m_Swapchain);
		}
		Swapchain::~Swapchain()
		{
			//CleanUp();
		}
		



	vk::SurfaceFormatKHR GetSurfaceFormat(vk::Format format, vk::ColorSpaceKHR colorSpace, SwapChainSupportDetails& support)
	{
		for (vk::SurfaceFormatKHR formatSupport : support.AvailableFormats)
		{
			if (formatSupport.format == format
				&& formatSupport.colorSpace == colorSpace) {
				return formatSupport;
			}
		}

		return support.AvailableFormats[0];
	}

	vk::PresentModeKHR GetPresentMode(vk::PresentModeKHR presentMode, SwapChainSupportDetails& support)
	{
		for (vk::PresentModeKHR presentModeSupport : support.AvailablePresentModes)
		{
			if (presentMode == presentModeSupport) {
				return presentModeSupport;
			}
		}

		return vk::PresentModeKHR::eFifo;
	}
	void CleanUp()
	{
	}
	vk::Extent2D GetSwapchainExtent(uint32_t width, uint32_t height, SwapChainSupportDetails& support) {

		auto capabilities = support.AvailableCapabilities;
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