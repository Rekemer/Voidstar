
#include "Prereq.h"
#include "RenderPass.h"
#include "Device.h"
#include "Log.h"
#include "RenderContext.h"
#include "Initializers.h"
#include "AttachmentManager.h"
#include "Image.h"
#include<algorithm>
namespace Voidstar
{





	void RenderPassBuilder::SetLoadOp(vk::AttachmentLoadOp loadOp)
	{
		m_LoadOp = loadOp;
	}
	void RenderPassBuilder::SetSaveOp(vk::AttachmentStoreOp storeOp)
	{
		m_StoreOp = storeOp;
	}
	void RenderPassBuilder::SetStencilLoadOp(vk::AttachmentLoadOp storeOp)
	{
		m_StencilLoadOp = storeOp;
	}
	void RenderPassBuilder::SetStencilSaveOp(vk::AttachmentStoreOp storeOp)
	{
		m_StencilStoreOp = storeOp;
	}
	void RenderPassBuilder::SetInitialLayout(vk::ImageLayout initial)
	{
		m_InitialLayout = initial;
	}
	void RenderPassBuilder::SetFinalLayout(vk::ImageLayout final)
	{
		m_FinalLayout = final;
	}

	vk::AttachmentDescription RenderPassBuilder::BuildAttachmentDesc()
	{
		auto desc = AttachmentDescription(m_Format,
			m_Samples, m_LoadOp,
			m_StoreOp, m_StencilLoadOp,
			m_StencilStoreOp, m_InitialLayout,
			m_FinalLayout);
		AddAttachment(desc);
		return desc;
	}

	void RenderPassBuilder::ColorOutput(std::string_view attachmentName,
		AttachmentManager& manager, vk::ImageLayout referenceLayout)
	{
		auto views = manager.GetColor({ attachmentName });
		m_Color.push_back(views);
		m_ColorReferences.push_back({ (uint32_t)m_OutputTypes.size(), referenceLayout});
		m_OutputTypes.push_back(OutputType::COLOR);
		m_Format = views[0]->SwapchainImage::GetFormat();
		m_Samples = views[0]->SwapchainImage::GetSample();
	}
	//void RenderPassBuilder::DepthOutput(std::string_view attachmentName,
	//	AttachmentManager& manager, vk::Format format, size_t width, size_t height,
	//	vk::SampleCountFlagBits samples,
	//	vk::ImageUsageFlags usage)
	//{
	//	assert(false);
	//}
		
	void RenderPassBuilder::DepthStencilOutput(std::string_view attachmentName, AttachmentManager& manager, vk::ImageLayout referenceLayout)
	{
		auto views = manager.GetDepth({ attachmentName });
		m_DepthStencil = views;
		m_DepthReferences.push_back({ (uint32_t)m_OutputTypes.size(), referenceLayout });
		m_OutputTypes.push_back(OutputType::DEPTH);
		m_Format = views[0]->SwapchainImage::GetFormat();
		m_Samples = views[0]->SwapchainImage::GetSample();
	}
	//void RenderPassBuilder::StencilOutput(std::string_view attachmentName, AttachmentSpec& spec, AttachmentManager& manager) {
	//}
	//void RenderPassBuilder::ResolveOutput(std::string_view attachmentName, AttachmentSpec& spec, AttachmentManager& manager) {
	//}
	void RenderPassBuilder::ResolveOutput(std::string_view attachmentName, AttachmentManager& manager, vk::ImageLayout referenceLayout)
	{
		auto views = manager.GetResolve({ attachmentName });
		m_Resolve = views;

		m_ResolveReferences.push_back({ (uint32_t)m_OutputTypes.size(), referenceLayout });
		m_OutputTypes.push_back(OutputType::RESOLVE);
		m_Format = views[0]->SwapchainImage::GetFormat();
		m_Samples = views[0]->SwapchainImage::GetSample();
	}

	

	//On tile - based - renderer, which is pretty much anything on mobile,
	//using input attachments is faster than the traditional multi - pass approach as pixel reads are fetched from tile memory instead of mainframebuffer, 
	//so if you target the mobile market it’s always a good idea to use input attachments instead of multiple passes when possible.


	/*No.The attachment reference layout tells Vulkan what layout to transition the image to at 
	the beginning of the subpass for which this reference is defined.Or more to the point
	, it is the layout which the image will be in for the duration of the subpass.
	*

		The first transition for an attached image of a render pass will be from the initialLayout
		for the render pass to the reference layout for the first subpass that uses the image.
		The last transition for an attached image will be from reference layout of the final subpass
		that uses the attachment to the finalLayout for the render pass.*/


	void RenderPassBuilder::AddAttachment(vk::AttachmentDescription desc)
	{
		m_Attachments.push_back(desc);
	}
	void RenderPassBuilder::AddSubpass(std::vector<int> indexColor, std::vector<int> indexDepth,
		std::vector<int> indexResolve)
	{
		AddSubpass(SubpassDescription(m_ColorReferences.size(), m_ColorReferences.data(), m_ResolveReferences.data(), m_DepthReferences.data()));
	}
	void RenderPassBuilder::AddSubpass(vk::SubpassDescription subpass)
	{
		m_Subpasses.push_back(subpass);
	}

	void RenderPassBuilder::AddSubpassDependency(vk::SubpassDependency subpassDependency)
	{
		m_Dependencies.push_back(subpassDependency);
	}
	UPtr<RenderPass> RenderPassBuilder::Build(std::string_view name, AttachmentManager& manager, size_t framebufferAmount)
	{
		auto device = RenderContext::GetDevice();
		//Now create the renderpass
		vk::RenderPassCreateInfo renderpassInfo = {};
		std::vector<vk::AttachmentDescription> attachments;
		for (auto attachment : m_Attachments)
		{
			attachments.push_back(attachment);
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
			auto vkRenderPass = device->GetDevice().createRenderPass(renderpassInfo);
			UPtr<RenderPass> renderPass = CreateUPtr<RenderPass>(name,vkRenderPass);
			renderPass->m_Framebuffers.resize(framebufferAmount);
			for (int i=0; i < framebufferAmount; i++)
			{
				int colorOutputOverall = 0;
				std::vector<vk::ImageView> views;
				for (auto type : m_OutputTypes)
				{
					if (type == OutputType::COLOR)
					{
						views.push_back(m_Color[colorOutputOverall][i]->GetImageView());
						colorOutputOverall++;
					}
					else if (type == OutputType::DEPTH)
					{
						views.push_back(m_DepthStencil[i]->GetImageView());
					}
					else if (type == OutputType::RESOLVE)
					{
						views.push_back(m_Resolve[i]->GetImageView());

					}
				}
				// width and height for all images are supposed to be equal to each other
				renderPass->m_Framebuffers[i] =  CreateFramebuffer(views, vkRenderPass, m_Color[0][0]->GetWidth(), m_Color[0][0]->GetHeight());
			}

			m_DepthReferences.clear();
			m_ColorReferences.clear();
			m_ResolveReferences.clear();
			m_DepthStencil.clear();
			m_Color.clear();
			m_Resolve.clear();
			return std::move(renderPass);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to create renderpass!");
		}
	}

	RenderPass::~RenderPass()
	{
		auto device = RenderContext::GetDevice()->GetDevice();
		device.destroyRenderPass(m_RenderPass);
	}

}

