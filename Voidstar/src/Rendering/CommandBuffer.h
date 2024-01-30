#pragma once
#include"vulkan/vulkan.hpp"
namespace Voidstar
{
	class Buffer;
	class IndexBuffer;
	class Image;
	class CommandBuffer
	{
	public:
		CommandBuffer() = default;
		CommandBuffer(CommandBuffer& commandBuffer)
		{
			m_CommandPool = commandBuffer.m_CommandPool;
			m_CommandBuffer= commandBuffer.m_CommandBuffer;
		}
		
		CommandBuffer(CommandBuffer&& commandBuffer) noexcept
			: m_CommandPool(commandBuffer.m_CommandPool),
			m_CommandBuffer(commandBuffer.m_CommandBuffer)
		{
			commandBuffer.m_CommandPool = nullptr;
			commandBuffer.m_CommandBuffer = nullptr;
		}
		CommandBuffer(vk::CommandBuffer&& commandBuffer)
		{
			m_CommandBuffer = commandBuffer;
			commandBuffer = nullptr;
		}
		CommandBuffer& operator=(CommandBuffer&& other) noexcept
		{
			if (this == &other) {
				return *this;
			}
			m_CommandPool = std::move(other.m_CommandPool);
			m_CommandBuffer = std::move(other.m_CommandBuffer);
			other.m_CommandPool = nullptr;
			other.m_CommandBuffer = nullptr;
			return *this;
		}
		
		//rendering 
		void BeginRendering();
	
		void BeginRenderPass(vk::RenderPass& renderPass, vk::Framebuffer& framebuffer, vk::Extent2D& extent, std::vector<vk::ClearValue>& clearValues);
	
		void EndRenderPass();
		void Submit(vk::Semaphore* waitSemaphores, vk::Semaphore* signalSemaphores, vk::Fence* fence);
		void Submit(vk::Fence* fence);
	
		void EndRendering();
		void Free();
		static CommandBuffer CreateBuffer(vk::CommandPool commandPool, vk::CommandBufferLevel level);
		static std::vector<CommandBuffer> CreateBuffers(vk::CommandPool commandPool, vk::CommandBufferLevel level, uint32_t count);

		// transfering operations
		vk::CommandBuffer BeginTransfering();
		
		void Transfer(Buffer* src, Buffer* target, void* data, size_t  dataSize);
		void MemBufferBarrier(vk::Buffer& buffer,size_t size, 
			vk::PipelineStageFlags srcPip,
			vk::AccessFlags src,
			vk::PipelineStageFlags dstPip,
			vk::AccessFlags dst)
		{
			vk::BufferMemoryBarrier barrier{};
			barrier.buffer= buffer;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.srcAccessMask = src;
			barrier.dstAccessMask = dst;
			barrier.size = size;
			m_CommandBuffer.pipelineBarrier(srcPip, dstPip, vk::DependencyFlags(),nullptr, barrier,nullptr);
			
		}
		void ChangeImageLayout(Image* image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, int mipMap = 1, int layers = 1);
		void ChangeImageLayoutRaw(vk::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, int mipMap = 1, int layers = 1);

		void CopyBufferToImage(Buffer& buffer, vk::Image& image, int width, int height, vk::Offset3D offset = {0,0,0}, int layers = 1);
		void CopyImageToBuffer(SPtr<Image> image, SPtr<Buffer> buffer);
		void EndTransfering();
		void SubmitSingle(vk::Fence fence);
		void SubmitSingle();
		vk::CommandBuffer& GetCommandBuffer() { return m_CommandBuffer; }

		~CommandBuffer();
	private:
		friend class Renderer;
		vk::CommandPool m_CommandPool;
		vk::CommandBuffer m_CommandBuffer;
	};
}