#pragma once
#include "vulkan/vulkan.hpp"
#include "RenderContext.h"
inline vk::VertexInputBindingDescription VertexBindingDescription(uint32_t binding,
	uint32_t stride,
	vk::VertexInputRate inputRate)
{
	vk::VertexInputBindingDescription vInputBindDescription{};
	vInputBindDescription.binding = binding;
	vInputBindDescription.stride = stride;
	vInputBindDescription.inputRate = inputRate;
	return vInputBindDescription;
}

inline vk::DescriptorSetLayoutBinding DescriptorBindingDescription(
	uint32_t binding,
	vk::DescriptorType type,
	vk::Flags<vk::ShaderStageFlagBits> shaderStages,
	uint32_t descriptorAmount)
{
	vk::DescriptorSetLayoutBinding layoutBinding;
	layoutBinding.binding = binding;
	layoutBinding.descriptorType = type;
	layoutBinding.stageFlags = shaderStages;
	layoutBinding.descriptorCount = descriptorAmount;
	return layoutBinding;
}



//// define order of subpasses?
//// where srcAccess mask operations occurr
////list operatoins that must be completed before staring render pass
//dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
////dependency.srcAccessMask = vk::AccessFlagBits::eNone;
//dependency.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
//// define rights of subpasses?
//dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
//dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

inline vk::SubpassDependency SubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass, vk::PipelineStageFlags srcStageMask, vk::AccessFlags srcAccessMask, vk::PipelineStageFlags dstStageMask, vk::AccessFlagBits dstcAccessMask)
{
	vk::SubpassDependency dependency{};
	dependency.srcSubpass = srcSubpass;
	dependency.dstSubpass = dstSubpass;
	dependency.srcStageMask = srcStageMask;
	dependency.srcAccessMask = srcAccessMask;
	dependency.dstStageMask = dstStageMask;
	dependency.dstAccessMask = dstcAccessMask;
	return dependency;
}

inline vk::SubpassDescription SubpassDescription(uint32_t colorAttachemntCount, vk::AttachmentReference* color, vk::AttachmentReference* resolve =nullptr,  vk::AttachmentReference* depthStencil = nullptr)
{
	vk::SubpassDescription subpass;
	subpass.flags = vk::SubpassDescriptionFlags();
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = colorAttachemntCount;
	subpass.pColorAttachments = color;
	subpass.pResolveAttachments = resolve;
	subpass.pInputAttachments= nullptr;
	subpass.inputAttachmentCount= 0;
	subpass.pDepthStencilAttachment = depthStencil;
	return subpass;
}


inline vk::Framebuffer CreateFramebuffer(std::vector < vk::ImageView> attachments, vk::RenderPass renderPass,size_t width, size_t height )
{
	auto device = Voidstar::RenderContext::GetDevice();
	vk::FramebufferCreateInfo framebufferInfo;
	framebufferInfo.flags = vk::FramebufferCreateFlags();
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.attachmentCount = attachments.size();
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = width;
	framebufferInfo.height = height;
	framebufferInfo.layers = 1;

	return device->GetDevice().createFramebuffer(framebufferInfo);
}

inline vk::AttachmentDescription AttachmentDescription(vk::Format swapchainImageFormat, vk::SampleCountFlagBits samples, vk::AttachmentLoadOp loadOp, vk::AttachmentStoreOp storeOp, vk::AttachmentLoadOp stencilLoadOp, vk::AttachmentStoreOp stencilStoreOp, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout)
{
	vk::AttachmentDescription attachment = {};
	attachment.flags = vk::AttachmentDescriptionFlags();
	attachment.format = swapchainImageFormat;
	attachment.samples = samples;
	attachment.loadOp = loadOp;
	attachment.storeOp = storeOp;
	attachment.stencilLoadOp = stencilLoadOp;
	attachment.stencilStoreOp = stencilStoreOp;
	attachment.initialLayout = initialLayout;
	attachment.finalLayout = finalLayout;
	return attachment;
}




inline vk::VertexInputAttributeDescription VertexInputAttributeDescription(
	uint32_t binding,
	uint32_t location,
	vk::Format format,
	uint32_t offset)
{
	vk::VertexInputAttributeDescription vInputAttribDescription{};
	vInputAttribDescription.location = location;
	vInputAttribDescription.binding = binding;
	vInputAttribDescription.format = format;
	vInputAttribDescription.offset = offset;
	return vInputAttribDescription;
}

inline vk::DescriptorImageInfo DescriptorImageInfo(
	vk::ImageLayout imageLayout,
	vk::ImageView imageView,
	vk::Sampler samlper)
{
	vk::DescriptorImageInfo imageDescriptor;
	imageDescriptor.imageLayout = imageLayout;
	imageDescriptor.imageView = imageView;
	imageDescriptor.sampler = samlper;
	return imageDescriptor;
}

