#include "Prereq.h"
#include "RenderPass.h"
#include "Device.h"
#include "Log.h"
namespace Voidstar
{
	//On tile - based - renderer, which is pretty much anything on mobile,
	//using input attachments is faster than the traditional multi - pass approach as pixel reads are fetched from tile memory instead of mainframebuffer, 
	//so if you target the mobile market it’s always a good idea to use input attachments instead of multiple passes when possible.








	/*No.The attachment reference layout tells Vulkan what layout to transition the image to at the beginning of the subpass for which this reference is defined.Or more to the point, it is the layout which the image will be in for the duration of the subpass.
	*

		The first transition for an attached image of a render pass will be from the initialLayout
		for the render pass to the reference layout for the first subpass that uses the image.
		The last transition for an attached image will be from reference layout of the final subpass
		that uses the attachment to the finalLayout for the render pass.*/


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

