#pragma once
#include "vulkan/vulkan.hpp"
#include <vector>
#include <utility>
namespace Voidstar
{
	class Device;
	class RenderPassBuilder
	{
	public:
		void AddAttachment(std::pair<vk::AttachmentDescription, vk::AttachmentReference> attachmment);
		void AddSubpass(vk::SubpassDescription subpass);
		void AddSubpassDependency(vk::SubpassDependency subpassDependency);
		vk::RenderPass Build(Device& device);
	private:
		std::vector<std::pair<vk::AttachmentDescription, vk::AttachmentReference>>m_Attachments;
		std::vector<vk::SubpassDescription> m_Subpasses;
		std::vector<vk::SubpassDependency> m_Dependencies;
	};
}
