#include "Prereq.h"
#include "AttachmentManager.h"
#include "RenderContext.h"
#include "SupportStruct.h"
#include "Device.h"
namespace Voidstar
{
	void AttachmentManager::CreateColor(std::string_view attachmentName,
		AttachmentManager& manager, vk::Format format, size_t width, size_t height,
		vk::SampleCountFlagBits samples,
		vk::ImageUsageFlags usage)
	{
		AttachmentSpec msaa;
		msaa.Specs.width = width;
		msaa.Specs.height = height;
		msaa.Specs.usage = usage;
		msaa.Specs.format = format;
		msaa.Specs.minFilter = vk::Filter::eLinear;
		msaa.Specs.magFilter = vk::Filter::eLinear;
		msaa.Specs.tiling = vk::ImageTiling::eOptimal;
		msaa.Specs.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		msaa.Amount = 3;
		msaa.Samples = samples;

		auto specs = msaa.Specs;

		std::vector<SPtr<Image>> images;
		images.resize(msaa.Amount);
		for (int i = 0; i < images.size(); i++)
		{
			auto msaaImage = Image::CreateVKImage(specs, msaa.Samples);
			auto msaaImageMemory = Image::CreateMemory(msaaImage, specs);
			auto msaaImageView = Image::CreateImageView(msaaImage, specs.format, specs.imageAspect);
			images[i] = CreateSPtr<Image>();
			images[i]->SetFormat(specs.format);
			images[i]->SetSample(msaa.Samples);
			images[i]->SetView(msaaImageView);
			images[i]->SetMemory(msaaImageMemory);
			images[i]->SetImage(msaaImage);
			images[i]->SetWidth(width);
			images[i]->SetHeight(height);
		}
		m_Color[attachmentName.data()] = images;
	}
	void AttachmentManager::CreateDepthStencil(std::string_view attachmentName,
		AttachmentManager& manager, size_t width, size_t height,
		vk::SampleCountFlagBits samples,
		vk::ImageUsageFlags usage)
	{
		DepthStencilSpecs depthSpec;
		depthSpec.Candidates = { vk::Format::eD32SfloatS8Uint ,
			vk::Format::eD24UnormS8Uint,
		vk::Format::eD16UnormS8Uint };

		depthSpec.Specs.width = width;
		depthSpec.Specs.height = height;
		depthSpec.Specs.usage = usage;
		depthSpec.Specs.tiling = vk::ImageTiling::eOptimal;
		depthSpec.Specs.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		depthSpec.Amount = 3;
		depthSpec.Samples = samples;
		depthSpec.FormatFeature = vk::FormatFeatureFlagBits::eDepthStencilAttachment;
		depthSpec.Specs.imageAspect = vk::ImageAspectFlagBits::eDepth;

		auto& depth = static_cast<DepthStencilSpecs&>(depthSpec);
		auto device = RenderContext::GetDevice();

		auto depthFormat = Image::GetFormat(
			device->GetDevicePhys(),
			depth.Candidates,
			depth.Specs.tiling,
			depth.FormatFeature
		);
		std::vector<SPtr<Image>> images;
		depth.Specs.format = depthFormat;
		images.resize(depth.Amount);
		for (int i = 0; i < depth.Amount; i++)
		{
			auto vkImage = Image::CreateVKImage(depth.Specs, depth.Samples);
			auto imageMemory = Image::CreateMemory(vkImage, depth.Specs);
			auto imageView = Image::CreateImageView(vkImage, depthFormat, depth.Specs.imageAspect);
			images[i] = CreateSPtr<Image>();
			images[i]->SetFormat(depth.Specs.format);
			images[i]->SetSample( depth.Samples);
			images[i]->SetView(imageView);
			images[i]->SetMemory(imageMemory);
			images[i]->SetImage(vkImage);
			images[i]->SetWidth(width);
			images[i]->SetHeight(height);
			
		}
		m_DepthStencil[attachmentName.data()] = images;
	}
	void AttachmentManager::CreateResolve(std::string_view attachmentName,
		AttachmentManager& manager, vk::Format format, size_t width, size_t height,
		vk::PresentModeKHR presentMode,
		vk::ColorSpaceKHR colorSpace)
	{
		SwapchainSpec resolveSpec;
		resolveSpec.Specs.width = width;
		resolveSpec.Specs.height = height;
		resolveSpec.Specs.usage = vk::ImageUsageFlagBits::eColorAttachment;
		resolveSpec.ColorSpace = colorSpace;
		resolveSpec.PresentMode = presentMode;
		resolveSpec.Specs.format = format;
		resolveSpec.Amount = 3;

		auto& swapchainSpec = static_cast<SwapchainSpec&>(resolveSpec);
		SwapChainSupportDetails support;
		auto device = RenderContext::GetDevice();
		auto surface = RenderContext::GetSurface();
		support.AvailableCapabilities = device->GetDevicePhys().getSurfaceCapabilitiesKHR(*surface);
		support.AvailablePresentModes = device->GetDevicePhys().getSurfacePresentModesKHR(*surface);
		support.AvailableFormats = device->GetDevicePhys().getSurfaceFormatsKHR(*surface);
		support.ViewportWidth = swapchainSpec.Specs.width;
		support.ViewportHeight = swapchainSpec.Specs.height;
		support.PresentMode = swapchainSpec.PresentMode;
		support.Format = swapchainSpec.Specs.format;
		support.ColorSpace = swapchainSpec.ColorSpace;
		support.Usage = swapchainSpec.Specs.usage;
		support.FrameAmount = swapchainSpec.Amount;
		RenderContext::CreateSwapchain(support);
		auto swapChain = RenderContext::GetSwapchain();
		auto& images = swapChain->GetImages();
		m_Resolve[attachmentName.data()] = images;
	}
}