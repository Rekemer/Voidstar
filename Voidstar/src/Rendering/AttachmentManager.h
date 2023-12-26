#pragma once
#include "Prereq.h"
#include "unordered_map"
#include "vulkan/vulkan.hpp"
#include "Image.h"
#include "SupportStruct.h"
#include "Swapchain.h"
namespace Voidstar
{

	struct AttachmentSpec
	{
		ImageSpecs Specs;
		// desired 
		size_t Amount = 3;
		vk::SampleCountFlagBits Samples;
	};
	struct SwapchainSpec : AttachmentSpec
	{
		vk::PresentModeKHR PresentMode;
		vk::ColorSpaceKHR ColorSpace;
	};
	struct DepthStencilSpecs : AttachmentSpec
	{
		// from most desirable to least desirable
		std::vector<vk::Format> Candidates;
		vk::FormatFeatureFlags  FormatFeature;
	};


	


	class AttachmentManager
	{
	public:
		std::vector<Image*> GetColor(std::vector< std::string_view> names)
		{
			std::vector<Image*> attachments;
			for (auto name : names)
			{
				auto& attachment = m_Color.at(name.data());
				for (auto& image : attachment)
				{
					
					attachments.push_back(image.get());
				
				}

			}
			return attachments;
		}
		std::vector<Image*> GetDepth(std::vector< std::string_view> names)
		{
			std::vector<Image*> attachments;
			for (auto name : names)
			{
				auto& attachment = m_DepthStencil.at(name.data());
				for (auto& image : attachment)
				{
				
					attachments.push_back(image.get());
				
				}

			}
			return attachments;
		}
		std::vector<SwapchainImage*> GetResolve(std::vector< std::string_view> names)
		{
			std::vector<SwapchainImage*> attachments;
			for (auto name : names)
			{
				auto& attachment = m_Resolve.at(name.data());
				for (auto& image : attachment)
				{
				
					attachments.push_back(image.get());
				
				}

			}
			return attachments;
		}
		std::vector<Attachment> GetAttachments(std::vector<std::string_view>& names,size_t amountPerFrameBuffer)
		{
			
		}
		void CreateColor(std::string_view attachmentName,
			AttachmentManager& manager, vk::Format format, size_t width, size_t height,
			vk::SampleCountFlagBits samples,
			vk::ImageUsageFlags usage);
		void CreateDepthStencil(std::string_view attachmentName,
			AttachmentManager& manager,  size_t width, size_t height,
			vk::SampleCountFlagBits samples,
			vk::ImageUsageFlags usage);
		void CreateResolve(std::string_view attachmentName,
			AttachmentManager& manager, vk::Format format, size_t width, size_t height,
			vk::PresentModeKHR presentMode,
			vk::ColorSpaceKHR colorSpace);

		void Destroy();
	private:
		// image or swapchain image
		std::unordered_map<std::string, std::vector<std::shared_ptr<Image>>> m_Color;
		std::unordered_map<std::string, std::vector<std::shared_ptr<SwapchainImage>>> m_Resolve;
		std::unordered_map<std::string, std::vector<std::shared_ptr<Image>>> m_DepthStencil;
	};
}
