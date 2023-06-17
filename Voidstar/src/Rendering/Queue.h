#pragma once
#include"vulkan/vulkan.hpp"
namespace Voidstar
{
	class Buffer;
	class IndexBuffer;
	class Queue
	{
	public:
		
		void CreateCommandPool();
		void CreateCommandBuffer();
		
		//rendering 
		void BeginRendering();
	
		void BeginRenderPass(vk::RenderPass* renderPass, vk::Framebuffer* framebuffer, vk::Extent2D* extent);
		void RecordCommand(Buffer* vertexBuffer, IndexBuffer* indexBuffer, vk::Pipeline* m_Pipeline, vk::PipelineLayout* m_PipelineLayout, vk::DescriptorSet* descriptorSet, vk::DescriptorSet* descriptorSetTex);
		void EndRenderPass();
		void Submit(vk::Semaphore* waitSemaphores, vk::Semaphore* signalSemaphores, vk::Fence* fence);
	
		void EndRendering();


		// transfering operations
		vk::CommandBuffer BeginTransfering();
		
		void Transfer(Buffer* src, Buffer* target, void* data, size_t  dataSize);
		
		void ChangeImageLayout(vk::Image* image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, int mipMap = 1);

		void CopyBufferToImage(Buffer* buffer,vk::Image* image, int width, int height);

		void EndTransfering();

		

		~Queue();
	private:
		vk::CommandPool m_CommandPool;
		vk::CommandBuffer m_CommandBuffer;
	};
}