#pragma once
#include <Prereq.h>
#include "vulkan/vulkan.hpp"
#include <vector>
#include <utility>
namespace Voidstar
{
	enum class AttachmentToFrameBuffer
	{
		// means if there are 3 attachments of type
		// then each frame buffer will have 1 attachment
		PER_FRAME_BUFFER,
		// means that all 3 attachments are covered 
		// by frame buffer
		ALL_FRAME_BUFFER
	};
	class Device;
	class RenderPass;
	class AttachmentManager;
	class FramebufferManager;
	class AttachmentSpec;
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
		
		void ColorOutput(std::string_view attachmentName, AttachmentManager&);
		void DepthStencilOutput(std::string_view attachmentName, AttachmentManager&);
		void ResolveOutput(std::string_view attachmentName, AttachmentManager&);

		void AddOutput(std::string_view name)
		{
			m_Color.push_back(name);
		};
		void AddAttachment(vk::AttachmentDescription description,vk::AttachmentReference attachmment);
		void AddSubpass(vk::SubpassDescription subpass);
		void AddSubpassDependency(vk::SubpassDependency subpassDependency);
		UPtr<RenderPass> Build(std::string_view name, AttachmentManager& manager,
			size_t frameBufferAmount);
	private:
		size_t m_FrameBufferAmount;
		std::vector<std::string_view> m_Color;
		std::vector<std::string_view> m_DepthStencil;
		std::vector<std::string_view> m_Resolve;
		vk::Format m_Format;
		vk::SampleCountFlagBits m_Samples;
		vk::AttachmentLoadOp m_LoadOp  ;
		vk::AttachmentStoreOp m_StoreOp;
		vk::AttachmentLoadOp m_StencilLoadOp;
		vk::AttachmentStoreOp m_StencilStoreOp;
		vk::ImageLayout m_InitialLayout;
		vk::ImageLayout m_FinalLayout;
		vk::AttachmentDescription m_Description = {};
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
		std::vector<vk::Framebuffer> m_Framebuffers;
	};
}
