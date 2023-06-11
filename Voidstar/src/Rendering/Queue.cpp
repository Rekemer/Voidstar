#include"Prereq.h"
#include"Queue.h"
#include"RenderContext.h"
#include"Device.h"
#include"Buffer.h"
#include"Log.h"
namespace Voidstar
{
	void Queue::CreateCommandPool()
	{
		auto device = RenderContext::GetDevice();
		vk::CommandPoolCreateInfo poolInfo;
		poolInfo.flags = vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		poolInfo.queueFamilyIndex = device->GetGraphicsIndex();
		try
		{
			m_CommandPool = device->GetDevice().createCommandPool(poolInfo);

		}
		catch (vk::SystemError err)
		{

			Log::GetLog()->error("Failed to create Command Pool");
		}
	}
	void Queue::CreateCommandBuffer()
	{
		auto device = RenderContext::GetDevice();
		vk::CommandBufferAllocateInfo allocInfo = {};
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = 1;
		try
		{
			m_CommandBuffer = device->GetDevice().allocateCommandBuffers(allocInfo)[0];
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to allocate  command buffer ");
		}
	}
	void Queue::BeginRendering()
	{
		m_CommandBuffer.reset();
	}

	void Queue::BeginRenderPass(vk::RenderPass* renderPass, vk::Framebuffer* framebuffer, vk::Extent2D* extent)
	{
		vk::CommandBufferBeginInfo beginInfo = {};


		m_CommandBuffer.begin(beginInfo);


		vk::RenderPassBeginInfo renderPassInfo = {};
		auto swapChainExtent = *extent;
		renderPassInfo.renderPass = *renderPass;
		renderPassInfo.framebuffer = *framebuffer;
		renderPassInfo.renderArea.offset.x = 0;
		renderPassInfo.renderArea.offset.y = 0;
		renderPassInfo.renderArea.extent = swapChainExtent;

		vk::ClearValue clearColor = { std::array<float, 4>{0.1f, .3f, 0.1f, 1.0f} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		m_CommandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
	}

	void Queue::RecordCommand(Buffer* vertexBuffer, Buffer* indexBuffer, vk::Pipeline* m_Pipeline, vk::PipelineLayout* m_PipelineLayout, vk::DescriptorSet* descriptorSet)
	{
		m_CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_PipelineLayout, 0, *descriptorSet, nullptr);
		m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_Pipeline);
		vk::DeviceSize offsets[] = { 0 };
		vk::Buffer vertexBuffers[] = { vertexBuffer->GetBuffer() };
		m_CommandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		m_CommandBuffer.bindIndexBuffer(indexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);
		//m_CommandBuffer.draw(6, 1, 0, 0);
		//m_CommandBuffer.draw(23, 1, 0, 0);
		m_CommandBuffer.drawIndexed(6, 1, 0, 0, 0);
	}

	void Queue::EndRenderPass()
	{
		m_CommandBuffer.endRenderPass();

		m_CommandBuffer.end();
	}

	void Queue::Submit(vk::Semaphore* waitSemaphores, vk::Semaphore* signalSemaphores, vk::Fence* fence)
	{
		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		vk::SubmitInfo submitInfo = {};

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		auto device = RenderContext::GetDevice();
		device->GetGraphicsQueue().submit(submitInfo, *fence);
	}
	void Queue::EndRendering()
	{
	}


	void Queue::BeginTransfering()
	{
		m_CommandBuffer.reset();

		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		m_CommandBuffer.begin(beginInfo);
	}
	void Queue::Transfer(Buffer* src, Buffer* target,void* data,size_t  dataSize)
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
	void Queue::EndTransfering()
	{
		m_CommandBuffer.end();
		auto device = RenderContext::GetDevice();

		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;
		device->GetGraphicsQueue().submit(1, &submitInfo, nullptr);

		device->GetGraphicsQueue().waitIdle();

	}
	Queue::~Queue()
	{
		auto device = RenderContext::GetDevice();
		device->GetDevice().destroyCommandPool(m_CommandPool);
	}
}