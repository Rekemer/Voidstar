#pragma once
#include "Prereq.h"
#include <vector>
#include "unordered_map"
#include "vulkan/vulkan.hpp"
#include "Image.h"
#include "SupportStruct.h"
#include "Swapchain.h"
#include "AttachmentSpec.h"
#include "Submission.h"

namespace Voidstar
{

	class AttachmentManager
	{
	public:
		void Init(const std::vector<TextureHandle>& handles, AttachmentHandle handle);
		std::vector<SPtr<Image>> GetColor(std::vector<AttachmentHandle> names)
		{
			auto attachments = GetAttachhmentsFrom(m_Color, names);
			return attachments;
		}
		std::vector<SPtr<Image>> GetDepth(std::vector<AttachmentHandle> names)
		{
			auto attachments = GetAttachhmentsFrom(m_DepthStencil, names);
			return attachments;
		}
		std::vector<SPtr<Image>> GetResolve(std::vector<AttachmentHandle> names)
		{
			auto attachments = GetAttachhmentsFrom(m_Resolve, names);
			return attachments;
		}
		
		void CreateColor(AttachmentHandle attachmentName,
			vk::Format format, size_t width, size_t height,
			vk::SampleCountFlagBits samples,
			vk::ImageUsageFlags usage, size_t attachmentAmount, vk::MemoryPropertyFlags flags = vk::MemoryPropertyFlagBits::eDeviceLocal);
		void CreateDepthStencil(AttachmentHandle attachmentName,
			size_t width, size_t height,
			vk::SampleCountFlagBits samples,
			vk::ImageUsageFlags usage,size_t attachmentAmount);
		
		TextureHandle GetColorTexture(AttachmentHandle handle, size_t frameNumber);

		void Destroy();
	private:
		std::vector<SPtr<Image>> GetAttachhmentsFrom(std::unordered_map<AttachmentHandle,
			std::vector<TextureHandle>>& from,
			std::vector<AttachmentHandle> names);
		
	private:
		// image or swapchain image
		Map<AttachmentHandle, std::vector<TextureHandle>> m_Color;
		Map<AttachmentHandle, std::vector<TextureHandle>> m_Resolve;
		Map<AttachmentHandle, std::vector<TextureHandle>> m_DepthStencil;

		
	};
}
