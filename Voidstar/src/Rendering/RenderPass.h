#pragma once
#include <Prereq.h>
#include "vulkan/vulkan.hpp"
#include <vector>
#include <utility>
namespace Voidstar
{
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
		
		void ColorOutput(std::string_view attachmentName, AttachmentManager&, vk::ImageLayout);
		void DepthStencilOutput(std::string_view attachmentName, AttachmentManager&, vk::ImageLayout);
		void ResolveOutput(std::string_view attachmentName, AttachmentManager&, vk::ImageLayout);

		void AddAttachment(vk::AttachmentDescription description);
		void AddSubpass(vk::SubpassDescription subpass);
		void AddSubpass(std::vector<int> indexColor,std::vector<int> indexDepth, std::vector<int> indexResolve);
		void AddSubpassDependency(vk::SubpassDependency subpassDependency);
		UPtr<RenderPass> Build(std::string_view name, AttachmentManager& manager,
			size_t frameBufferAmount);
	private:
		size_t m_FrameBufferAmount;
		std::vector<std::vector<Image*>> m_Color;
		std::vector<Image*> m_DepthStencil;
		std::vector<SwapchainImage*> m_Resolve;

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
		std::vector<vk::Framebuffer> m_Framebuffers;
	};
}
