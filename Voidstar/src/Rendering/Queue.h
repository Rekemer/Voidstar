#pragma once
#include"vulkan/vulkan.hpp"
namespace Voidstar
{
	class Buffer;
	class Queue
	{
	public:
		void CreateCommandPool();
		void CreateCommandBuffer();
		void BeginRendering();
		void BeginRenderPass(vk::RenderPass* renderPass, vk::Framebuffer* framebuffer, vk::Extent2D* extent);
		void RecordCommand(Buffer* vertexBuffer, Buffer* indexBuffer, vk::Pipeline* m_Pipeline, vk::PipelineLayout* m_PipelineLayout, vk::DescriptorSet* descriptorSet);
		void EndRenderPass();
		void Submit(vk::Semaphore* waitSemaphores, vk::Semaphore* signalSemaphores, vk::Fence* fence);
		void EndRendering();
		void BeginTransfering();
		void EndTransfering();
		void Transfer(Buffer* src, Buffer* target, void* data, size_t  dataSize);
		~Queue();
	private:
		vk::CommandPool m_CommandPool;
		vk::CommandBuffer m_CommandBuffer;
	};
}