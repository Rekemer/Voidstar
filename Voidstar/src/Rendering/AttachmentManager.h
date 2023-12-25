#pragma once
#include "Prereq.h"
#include "unordered_map"
#include "vulkan/vulkan.hpp"
#include "Image.h"
#include "SupportStruct.h"
#include "Swapchain.h"
namespace Voidstar
{
	struct IAttach
	{
		void virtual Destroy() = 0;
	};
	class Attachment
	{
	public:
		template<typename T>
		Attachment(T data) : m_Amount{ data.size() },
			m_Self{ CreateUPtr<Attachable<T>>(std::move(data)) }
		{

		}
		Attachment(Attachment& attach) : Attachment{std::move(attach)}
		{
		}
		Attachment(Attachment&& attach)
		{
			m_Self =  std::move(attach.m_Self);
			m_Amount=  attach.m_Amount;
		}
		~Attachment()
		{

		};

	private:
		UPtr<IAttach> m_Self;
		size_t m_Amount;
	};

	struct AttachmentSpec
	{
		ImageSpecs Specs;
		size_t Amount = 1;
		vk::SampleCountFlags Samples;
	};

	template<typename T>Attachment Attach(AttachmentSpec& Specs)
	{
		assert(false);
		return {}
	}
	template<>Attachment Attach<SwapchainImage>(AttachmentSpec& Specs)
	{
		SwapChainSupportDetails support;
		auto device = RenderContext::GetDevice();
		auto surface = RenderContext::GetSurface();
		support.AvailableCapabilities = device->GetDevicePhys().getSurfaceCapabilitiesKHR(*surface);
		support.AvailablePresentModes = device->GetDevicePhys().getSurfacePresentModesKHR(*surface);
		support.AvailableFormats = device->GetDevicePhys().getSurfaceFormatsKHR(*surface);
		support.ViewportWidth = Specs.Specs.width;
		support.ViewportHeight = Specs.Specs.height;
		support.PresentMode = vk::PresentModeKHR::eFifo;
		support.Format = vk::Format::eB8G8R8A8Unorm;
		support.ColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
		support.Usage = Specs.Specs.usage;
		support.FrameAmount = Specs.Amount;
		RenderContext::CreateSwapchain(support);
		auto swapChain = RenderContext::GetSwapchain();
		auto& images = swapChain->GetImages();
		return Attachment{ images };

	}
	template<>Attachment Attach<Image>(AttachmentSpec& Specs)
	{
		auto specs = Specs.Specs;
		auto samples = RenderContext::GetDevice()->GetSamples();
		auto msaaImage = Image::CreateVKImage(specs, samples);
		auto msaaImageMemory = Image::CreateMemory(msaaImage, specs);
		auto msaaImageView = Image::CreateImageView(msaaImage, specs.format, vk::ImageAspectFlagBits::eColor);

		m_PagesImageMSAA[i] = CreateSPtr<Image>();
		m_PagesImageMSAA[i]->SetFormat(swapchainFormat);
		m_PagesImageMSAA[i]->SetView(msaaImageView);
		m_PagesImageMSAA[i]->SetMemory(msaaImageMemory);
		m_PagesImageMSAA[i]->SetImage(msaaImage);

		return Attachment{ std::vector<Image>() };
	}
	
	template<typename T>
	class Attachable : public IAttach
	{
	public:
		Attachable(T data) : m_Data{std::move(data)}
		{

		}
		void Destroy() override
		{

		}
		T m_Data;
	};
	

	
	class AttachmentManager
	{
	public:
		template<typename T>
		void AddAttachment(std::string_view name, AttachmentSpec& spec)
		{
			auto attachment = Attach<T>(spec);
			m_Attachments.insert({ name.data() , std::move(Attachment(attachment)) });
		}
		void Destroy();
	private:
		// image or swapchain image
		std::unordered_map<std::string, Attachment> m_Attachments;
	};
}
