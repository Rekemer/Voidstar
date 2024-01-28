#pragma once
#include <Prereq.h>
#include "vulkan/vulkan.hpp"
#include <vector>
#include <functional>
#include "RenderContext.h"
#include "Device.h"
#include <utility>
#include "IExecute.h"
namespace Voidstar
{
	class CommandBuffer;
	
	enum class OutputType
	{
		COLOR,
		DEPTH,
		RESOLVE,
	};
	class Device;
	class RenderPass;
	class AttachmentManager;
	class FramebufferManager;
	class AttachmentSpec;
	class Image;
	class SwapchainImage;
	class RenderPassBuilder
	{
	public:
		// will get from vector
		void SetLoadOp(vk::AttachmentLoadOp loadOp);
		void SetSaveOp(vk::AttachmentStoreOp storeOp);
		void SetStencilLoadOp(vk::AttachmentLoadOp storeOp);
		void SetStencilSaveOp(vk::AttachmentStoreOp storeOp);
		void SetInitialLayout(vk::ImageLayout initial);
		void SetFinalLayout(vk::ImageLayout final);
		void SetFrameBufferAmount(size_t amount) { m_FrameBufferAmount = amount; }
		vk::AttachmentDescription BuildAttachmentDesc();
		

		void PresentOutput();
		void ColorOutput(std::string_view attachmentName, AttachmentManager&, vk::ImageLayout);
		void DepthStencilOutput(std::string_view attachmentName, AttachmentManager&, vk::ImageLayout);
		void ResolveOutput(std::string_view attachmentName, AttachmentManager&, vk::ImageLayout);

		void AddAttachment(vk::AttachmentDescription description);
		void AddSubpass(vk::SubpassDescription subpass);
		void AddSubpass(std::vector<int> indexColor,std::vector<int> indexDepth, std::vector<int> indexResolve);
		void AddSubpassDependency(vk::SubpassDependency subpassDependency);

		template<typename Func>
		UPtr<IExecute> Build(std::string_view name,
			AttachmentManager& manager,
			size_t framebufferAmount,
			vk::Extent2D extent,
			std::vector<vk::ClearValue> clearValues,
			Func execute)
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

				std::vector<vk::Framebuffer> framebuffers (framebufferAmount);
			try
			{
				auto vkRenderPass = device->GetDevice().createRenderPass(renderpassInfo);
				
				for (int i = 0; i < framebufferAmount; i++)
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
							views.push_back(m_DepthStencil[0]->GetImageView());
						}
						else if (type == OutputType::RESOLVE)
						{
							views.push_back(m_Resolve[i]->GetImageView());

						}
					}
					// width and height for all images are supposed to be equal to each other
					framebuffers[i] = CreateFramebuffer(views, vkRenderPass, m_Color[0][0]->GetWidth(), m_Color[0][0]->GetHeight());
				}

				UPtr<IExecute> renderPass = CreateUPtr<RenderPass>(name, vkRenderPass, extent, clearValues, execute, framebuffers);
				m_DepthReferences.clear();
				m_ColorReferences.clear();
				m_ResolveReferences.clear();
				m_DepthStencil.clear();
				m_Color.clear();
				m_Resolve.clear();
				m_IsMSAA = false;
				return std::move(renderPass);
			}
			catch (vk::SystemError err)
			{
				Log::GetLog()->error("Failed to create renderpass!");
			}
		}
	private:
		size_t m_FrameBufferAmount;
		std::vector<std::vector<SPtr<Image>>> m_Color;
		std::vector<SPtr<Image>> m_DepthStencil;
		std::vector<SPtr<Image>> m_Resolve;
		vk::Format m_Format;
		vk::SampleCountFlagBits m_Samples;
		vk::AttachmentLoadOp m_LoadOp  ;
		vk::AttachmentStoreOp m_StoreOp;
		vk::AttachmentLoadOp m_StencilLoadOp;
		vk::AttachmentStoreOp m_StencilStoreOp;
		vk::ImageLayout m_InitialLayout;
		vk::ImageLayout m_FinalLayout;
		vk::AttachmentDescription m_Description = {};
		std::vector<vk::AttachmentDescription> m_Attachments;


		std::vector<vk::AttachmentReference> m_ColorReferences;
		std::vector<vk::AttachmentReference> m_DepthReferences;
		std::vector<vk::AttachmentReference> m_ResolveReferences;
		std::vector<OutputType>m_OutputTypes;

		bool m_IsMSAA = false;
		std::vector<vk::SubpassDescription> m_Subpasses;
		std::vector<vk::SubpassDependency> m_Dependencies;
	};
	
	class RenderPass : public IExecute
	{
	public:
		RenderPass(std::string_view name, vk::RenderPass renderPass,
			vk::Extent2D extent, std::vector<vk::ClearValue> clearValues,
			Func execute,
			std::vector<vk::Framebuffer>& frameBuffers) : m_Name{name.data()},
			m_Extent{extent},
			m_ClearValues{ clearValues },
			m_RenderPass{ renderPass },
			m_Execute{execute},
			m_Framebuffers{ frameBuffers }

		{

		}
		RenderPass(RenderPass&& pass)
		{

		}
		RenderPass(const RenderPass& pass) = delete;
		RenderPass& operator=(const RenderPass& pass) = delete;
		
		RenderPass& operator=(RenderPass&& pass)
		{

		}
		void Execute(CommandBuffer& cmd, size_t frameIndex) override;
		
		vk::RenderPass GetRaw()
		{
			return m_RenderPass;
		}
		std::string_view GetName()
		{
			return m_Name.data();
		}
		~RenderPass();
	private:
		friend class RenderPassBuilder;
		std::string m_Name;
		vk::RenderPass m_RenderPass;
		Func m_Execute;
		vk::Extent2D m_Extent;
		std::vector<vk::ClearValue> m_ClearValues;
		std::vector<vk::Framebuffer> m_Framebuffers;
		
	};
}
