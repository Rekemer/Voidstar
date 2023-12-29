#pragma once
#include "Prereq.h"
#include "unordered_map"
#include "vulkan/vulkan.hpp"
#include "Image.h"
#include "SupportStruct.h"
#include "Swapchain.h"
#include "AttachmentSpec.h"
namespace Voidstar
{

	class AttachmentManager
	{
	public:
		AttachmentManager()
		{
			// swachain images can be targeted as color or resolve
			auto swapchainImages = RenderContext::GetFrames();
			m_Resolve["Default"]  = swapchainImages;
			m_Color["Default"]  = swapchainImages;
		}
		std::vector<SPtr<Image>> GetColor(std::vector< std::string_view> names)
		{
			auto attachments = GetAttachhmentsFrom(m_Color, names);
			return attachments;
		}
		std::vector<SPtr<Image>> GetDepth(std::vector< std::string_view> names)
		{
			auto attachments = GetAttachhmentsFrom(m_DepthStencil, names);
			return attachments;
		}
		std::vector<SPtr<Image>> GetResolve(std::vector< std::string_view> names)
		{
			auto attachments = GetAttachhmentsFrom(m_Resolve, names);
			return attachments;
		}
		void CreateColor(std::string_view attachmentName,
			AttachmentManager& manager, vk::Format format, size_t width, size_t height,
			vk::SampleCountFlagBits samples,
			vk::ImageUsageFlags usage,size_t attachmentAmount);
		void CreateDepthStencil(std::string_view attachmentName,
			AttachmentManager& manager,  size_t width, size_t height,
			vk::SampleCountFlagBits samples,
			vk::ImageUsageFlags usage,size_t attachmentAmount);
		
		void Destroy();
	private:
		std::vector<SPtr<Image>> GetAttachhmentsFrom(std::unordered_map<std::string, std::vector<SPtr<Image>>>& from,
			std::vector< std::string_view> names)
		{
			std::vector<SPtr<Image>> attachments;
			for (auto name : names)
			{
				auto& attachment = from.at(name.data());
				for (auto& image : attachment)
				{

					attachments.push_back(image);

				}

			}
			return attachments;
		}
	private:
		// image or swapchain image
		std::unordered_map<std::string, std::vector<SPtr<Image>>> m_Color;
		std::unordered_map<std::string, std::vector<SPtr<Image>>> m_Resolve;
		std::unordered_map<std::string, std::vector<SPtr<Image>>> m_DepthStencil;
	};
}
