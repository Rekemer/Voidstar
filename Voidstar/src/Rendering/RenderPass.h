#pragma once
#include <Prereq.h>
#include "vulkan/vulkan.hpp"
#include <vector>
#include <utility>
namespace Voidstar
{
	class Device;
	class RenderPass;
	class RenderPassBuilder
	{
	public:
		void AddAttachment(std::pair<vk::AttachmentDescription, vk::AttachmentReference> attachmment);
		void AddSubpass(vk::SubpassDescription subpass);
		void AddSubpassDependency(vk::SubpassDependency subpassDependency);
		UPtr<RenderPass> Build(std::string_view name,Device& device);
	private:
		std::vector<std::pair<vk::AttachmentDescription, vk::AttachmentReference>> m_Attachments;
		std::vector<vk::SubpassDescription> m_Subpasses;
		std::vector<vk::SubpassDependency> m_Dependencies;
	};
	class RenderPass
	{
	public:
		RenderPass(std::string_view name, vk::RenderPass renderPass) : m_Name{name.data()},
			m_RenderPass{ renderPass }
		{

		}
		vk::RenderPass GetRaw()
		{
			return m_RenderPass;
		}
		~RenderPass();
	private:
		friend class RenderPassBuilder;
		std::string m_Name;
		vk::RenderPass m_RenderPass;
	};
}
