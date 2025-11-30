#include"Prereq.h"
#include"CommandBuffer.h"
#include"RenderContext.h"
#include"Device.h"
#include"Buffer.h"
#include"IndexBuffer.h"
#include"CommandPoolManager.h"
#include"Renderer.h"
#include"Log.h"
#include "Image.h"
namespace Voidstar
{

	void CommandBuffer::BeginRendering()
	{
		m_CommandBuffer.reset();
		vk::CommandBufferBeginInfo beginInfo = {};
		m_CommandBuffer.begin(beginInfo);
	}

	void CommandBuffer::BeginRenderPass(vk::RenderPass& renderPass, vk::Framebuffer& framebuffer, vk::Extent2D& extent, std::vector<vk::ClearValue>& clearValues)
	{
		


		vk::RenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = framebuffer;
		renderPassInfo.renderArea.offset.x = 0;
		renderPassInfo.renderArea.offset.y = 0;
		renderPassInfo.renderArea.extent = extent;

		

		renderPassInfo.clearValueCount = clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();

		m_CommandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
	}

	

	void CommandBuffer::EndRenderPass()
	{
		m_CommandBuffer.endRenderPass();

	}


	void CommandBuffer::Submit(vk::Fence* fence)
	{
		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput};
		vk::SubmitInfo submitInfo = {};

		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;

		auto device = RenderContext::GetDevice();
		device->GetGraphicsQueue().submit(submitInfo, *fence);
	}

	void CommandBuffer::Submit(
		const std::vector<vk::Semaphore>& waitSemaphores,
		const std::vector<vk::Semaphore>& signalSemaphores,
		vk::Fence* fence,
		vk::TimelineSemaphoreSubmitInfo* timelineInfo)
	{
		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		vk::SubmitInfo submitInfo = {};
		submitInfo.pNext = timelineInfo;

		submitInfo.waitSemaphoreCount = waitSemaphores.size();
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;

		submitInfo.signalSemaphoreCount = signalSemaphores.size();
		submitInfo.pSignalSemaphores = signalSemaphores.data();
		auto device = RenderContext::GetDevice();
		device->GetGraphicsQueue().submit(submitInfo, *fence);
	}
	void CommandBuffer::EndRendering()
		{
		m_CommandBuffer.end();
	}

	void CommandBuffer::Free()
	{
		if (m_CommandPool)
		{
			RenderContext::GetDevice()->GetDevice().freeCommandBuffers(m_CommandPool, m_CommandBuffer);
		}
	}


	CommandBuffer CommandBuffer::CreateBuffer(vk::CommandPool commandPool, vk::CommandBufferLevel level)
	{
		CommandBuffer cmdBuffer;
		cmdBuffer.m_CommandPool = commandPool;
		vk::CommandBufferAllocateInfo cmdBufAllocateInfo;
		cmdBufAllocateInfo.commandPool = commandPool;
		cmdBufAllocateInfo.level = level;
		cmdBufAllocateInfo.commandBufferCount = 1;
		try
		{
			
			cmdBuffer.m_CommandBuffer = RenderContext::GetDevice()->GetDevice().allocateCommandBuffers(cmdBufAllocateInfo)[0];
			return cmdBuffer;
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to allocate command buffer");
		}
		
	}

	std::vector<CommandBuffer> CommandBuffer::CreateBuffers(vk::CommandPool commandPool, vk::CommandBufferLevel level, uint32_t count)
	{
		std::vector<CommandBuffer> commandBuffers;
		commandBuffers.reserve(count);
		vk::CommandBufferAllocateInfo allocInfo = {};
		allocInfo.commandPool = commandPool;
		allocInfo.level = level;
		allocInfo.commandBufferCount = count;
		try
		{
			auto vkCommandBuffers = RenderContext::GetDevice()->GetDevice().allocateCommandBuffers(allocInfo);

			for (auto& vkCommandBuffer : vkCommandBuffers)
			{
				commandBuffers.emplace_back(std::move(vkCommandBuffer));
			}
			return std::move(commandBuffers);

		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to allocate  command buffer ");
		}
	}


	 vk::CommandBuffer CommandBuffer::BeginTransfering()
	{


		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		m_CommandBuffer.begin(beginInfo);
		return m_CommandBuffer;
	}
	void CommandBuffer::Transfer(Buffer* src, Buffer* target,void* data,size_t  dataSize)
	{
		m_CommandBuffer.reset();

		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		m_CommandBuffer.begin(beginInfo);

		auto device = RenderContext::GetDevice();
		auto memory = device->GetDevice().mapMemory(src->GetMemory(), 0, dataSize);
		memcpy(memory, data, dataSize);

		device->GetDevice().unmapMemory(src->GetMemory());


		vk::BufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = dataSize;
		m_CommandBuffer.copyBuffer(src->GetBuffer(), target->GetBuffer(), copyRegion);

	}
	void CommandBuffer::ChangeImageLayout(Image* image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, int mipMap, int layers )
	{
		image->m_ImageLayout  = newLayout;
		ChangeImageLayoutRaw(image->m_Image, oldLayout, newLayout, mipMap, layers);
		
	}

	

	void CommandBuffer::ChangeImageLayoutRaw(vk::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, int mipMap,int layers)
	{
		/*
					typedef struct VkImageSubresourceRange {
					VkImageAspectFlags    aspectMask;
					uint32_t              baseMipLevel;
					uint32_t              levelCount;
					uint32_t              baseArrayLayer;
					uint32_t              layerCount;
				} VkImageSubresourceRange;
				*/
		if (oldLayout == newLayout) return;
		vk::ImageSubresourceRange access;
		access.aspectMask = vk::ImageAspectFlagBits::eColor;
		access.baseMipLevel = 0;
		access.levelCount = mipMap;
		access.baseArrayLayer = 0;
		access.layerCount = layers;

		/*
		typedef struct VkImageMemoryBarrier {
			VkStructureType            sType;
			const void* pNext;
			VkAccessFlags              srcAccessMask;
			VkAccessFlags              dstAccessMask;
			VkImageLayout              oldLayout;
			VkImageLayout              newLayout;
			uint32_t                   srcQueueFamilyIndex;
			uint32_t                   dstQueueFamilyIndex;
			VkImage                    image;
			VkImageSubresourceRange    subresourceRange;
		} VkImageMemoryBarrier;
		*/
		vk::ImageMemoryBarrier barrier;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange = access;

		vk::PipelineStageFlags sourceStage, destinationStage;

		if (oldLayout == vk::ImageLayout::eUndefined
			&& newLayout == vk::ImageLayout::eTransferDstOptimal) {

			barrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eGeneral)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eNone;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead;
			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eComputeShader;
		}
		else if (oldLayout == vk::ImageLayout::eGeneral && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			sourceStage = vk::PipelineStageFlagBits::eComputeShader;
			destinationStage = vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eTessellationEvaluationShader;
		}
		else if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			sourceStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && newLayout == vk::ImageLayout::eColorAttachmentOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
			sourceStage = vk::PipelineStageFlagBits::eFragmentShader;
			barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		}
		else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && newLayout == vk::ImageLayout::eTransferSrcOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			sourceStage = vk::PipelineStageFlagBits::eFragmentShader; 
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferSrcOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader; 
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eColorAttachmentOptimal)
		{
			barrier.srcAccessMask = {}; // 0

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		}
		else if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal && newLayout == vk::ImageLayout::eTransferSrcOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			sourceStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

			barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferSrcOptimal && newLayout == vk::ImageLayout::eColorAttachmentOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
			sourceStage = vk::PipelineStageFlagBits::eTransfer;

			barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = {}; // 0
			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;

			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eGeneral &&
			newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask =
				vk::AccessFlagBits::eShaderRead |
				vk::AccessFlagBits::eShaderWrite;

			sourceStage =
				vk::PipelineStageFlagBits::eComputeShader |
				vk::PipelineStageFlagBits::eFragmentShader |
				vk::PipelineStageFlagBits::eVertexShader; 

			barrier.dstAccessMask =
				vk::AccessFlagBits::eTransferWrite;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
			newLayout == vk::ImageLayout::eGeneral)
		{
			barrier.srcAccessMask =
				vk::AccessFlagBits::eTransferWrite;

			sourceStage =
				vk::PipelineStageFlagBits::eTransfer;
			barrier.dstAccessMask =
				vk::AccessFlagBits::eShaderRead |
				vk::AccessFlagBits::eShaderWrite;


			destinationStage = vk::PipelineStageFlagBits::eComputeShader |
				vk::PipelineStageFlagBits::eFragmentShader |
				vk::PipelineStageFlagBits::eVertexShader;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
			newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask =
				vk::AccessFlagBits::eTransferWrite;

			sourceStage =
				vk::PipelineStageFlagBits::eTransfer;

			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal &&
			newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
			sourceStage = vk::PipelineStageFlagBits::eFragmentShader;
			
			barrier.dstAccessMask =
				vk::AccessFlagBits::eTransferWrite;
			destinationStage =
				vk::PipelineStageFlagBits::eTransfer;

		}
		else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal &&
			newLayout == vk::ImageLayout::eGeneral)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
			sourceStage = vk::PipelineStageFlagBits::eFragmentShader;

			barrier.dstAccessMask =
				vk::AccessFlagBits::eShaderRead |
				vk::AccessFlagBits::eShaderWrite;


			destinationStage = vk::PipelineStageFlagBits::eComputeShader |
				vk::PipelineStageFlagBits::eFragmentShader |
				vk::PipelineStageFlagBits::eVertexShader;

		}
		else 
		{

			assert(false);
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		

		m_CommandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), nullptr, nullptr, barrier);
	}
	void CommandBuffer::CopyBufferToImage(Buffer& buffer, vk::Image image, int width, int height, int bufferOffset,vk::Offset3D offset,
		int baseLayer , int layerCount)
	{
		vk::BufferImageCopy copy;
		copy.bufferOffset = bufferOffset;
		copy.bufferRowLength = 0;
		copy.bufferImageHeight = 0;
		copy.imageOffset = offset;
		copy.imageExtent = vk::Extent3D(
			width,
			height,
			1.0f
		);
		vk::ImageSubresourceLayers access;
		access.aspectMask = vk::ImageAspectFlagBits::eColor;
		access.mipLevel = 0;
		access.baseArrayLayer = baseLayer;
		access.layerCount = layerCount;
		copy.imageSubresource = access;


		m_CommandBuffer.copyBufferToImage(
			buffer.GetBuffer(), image, vk::ImageLayout::eTransferDstOptimal, copy
		);
	}
	void CommandBuffer::CopyImageToBuffer(SPtr<Image> image, SPtr<Buffer> buffer)
	{
		auto oldLayout = image->GetLayout();
		ChangeImageLayout(image.get(),oldLayout, vk::ImageLayout::eTransferSrcOptimal);
		vk::BufferImageCopy copy;
		copy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		copy.imageSubresource.layerCount= 1;
		copy.imageExtent = vk::Extent3D{ (uint32_t)image->GetWidth(), (uint32_t)image->GetHeight(), 1};
		m_CommandBuffer.copyImageToBuffer(image->GetImage(), vk::ImageLayout::eTransferSrcOptimal, buffer->GetBuffer(), copy);
		ChangeImageLayout(image.get(), vk::ImageLayout::eTransferSrcOptimal,oldLayout);
	}
	void CommandBuffer::EndTransfering()
	{
		m_CommandBuffer.end();
		

	}
	void CommandBuffer::SubmitSingle(vk::Fence fence)
	{
		auto device = RenderContext::GetDevice();

		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;
		device->GetGraphicsQueue().submit(1, &submitInfo, fence);

		
	}
	void CommandBuffer::SubmitSingle()
	{
		auto device = RenderContext::GetDevice();

		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;
		device->GetGraphicsQueue().submit(1, &submitInfo, nullptr);
		device->GetGraphicsQueue().waitIdle();

	}
	CommandBuffer::~CommandBuffer()
	{
		//auto device = RenderContext::GetDevice();
		//Free();
	}
}