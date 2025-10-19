#pragma once
#include <Prereq.h>
#include "vulkan/vulkan.hpp"
#include <vector>
#include "RenderContext.h"
#include "Device.h"
#include "Image.h"
#include "Log.h"
#include <utility>
#include "IExecute.h"
#include "Initializers.h"
#include "Submission.h"
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
	class SwapchainImage;
	

	
	struct VOIDSTAR_API RenderPass 
	{
		vk::SampleCountFlagBits samples;
		vk::RenderPass m_RenderPass;
		vk::Extent2D m_Extent;
		std::vector<vk::ClearValue> m_ClearValues;
		FrameBufferHandle m_FrameBufferHandle;
	};

	class VOIDSTAR_API RenderPassBuilder
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
		void AddSubpass(std::vector<int> indexColor, std::vector<int> indexDepth, std::vector<int> indexResolve);
		void AddSubpassDependency(vk::SubpassDependency subpassDependency);

		RenderPass Build(
			AttachmentManager& manager,
			size_t framebufferAmount,
			vk::Extent2D extent,
			std::vector<vk::ClearValue> clearValues
		);

	private:
		size_t m_FrameBufferAmount;
		std::vector<std::vector<SPtr<Image>>> m_Color;
		std::vector<SPtr<Image>> m_DepthStencil;
		std::vector<SPtr<Image>> m_Resolve;
		vk::Format m_Format;
		vk::SampleCountFlagBits m_Samples;
		vk::AttachmentLoadOp m_LoadOp;
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


}
