#include "Prereq.h"
#include "RenderPass.h"
#include "Device.h"
#include "Log.h"
namespace Voidstar
{
	void RenderPassBuilder::AddAttachment(std::pair<vk::AttachmentDescription, vk::AttachmentReference> attachment)
	{
		m_Attachments.push_back(attachment);
	}

	void RenderPassBuilder::AddSubpass(vk::SubpassDescription subpass)
	{
		m_Subpasses.push_back(subpass);
	}

	void RenderPassBuilder::AddSubpassDependency(vk::SubpassDependency subpassDependency)
	{
		m_Dependencies.push_back(subpassDependency);
	}
	vk::RenderPass RenderPassBuilder::Build(Device& device)
	{
		//Now create the renderpass
		vk::RenderPassCreateInfo renderpassInfo = {};

		std::vector<vk::AttachmentDescription> attachments;
		for (auto attachment : m_Attachments)
		{
			attachments.push_back(attachment.first);
		}
		// to be able to map NDC to screen coordinates - Viewport ans Scissors Transform
		renderpassInfo.flags = vk::RenderPassCreateFlags();
		renderpassInfo.attachmentCount = attachments.size();
		renderpassInfo.pAttachments = attachments.data();
		renderpassInfo.subpassCount = m_Subpasses.size();
		renderpassInfo.pSubpasses = m_Subpasses.data();
		renderpassInfo.dependencyCount = m_Dependencies.size();
		renderpassInfo.pDependencies = m_Dependencies.data();

		try 
		{
			return device.GetDevice().createRenderPass(renderpassInfo);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to create renderpass!");
		}
	}

}

