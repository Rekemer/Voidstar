#pragma once
#include"vulkan/vulkan.hpp"
#include"RenderContext.h"
#include"Device.h"
namespace Voidstar
{
	class Fence
	{
	public:
		Fence()
		{
			vk::FenceCreateInfo fenceInfo = {};
			fenceInfo.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;
			m_Fence =RenderContext::GetDevice()->GetDevice().createFence(fenceInfo);
		};
		Fence(const Fence& fence) = delete;
		Fence(Fence&& fence)
		{
			m_Fence = fence.m_Fence;
			fence.m_Fence = VK_NULL_HANDLE;

		};
		~Fence()
		{
			Destroy();
		}
		void Destroy()
		{
			if (m_Fence != VK_NULL_HANDLE)
			{
				RenderContext::GetDevice()->GetDevice().waitIdle();
				RenderContext::GetDevice()->GetDevice().destroyFence(m_Fence);
				m_Fence = VK_NULL_HANDLE;
			}
		}
		vk::Fence GetFence()
		{

			return m_Fence;
		}
	private:
		vk::Fence m_Fence = VK_NULL_HANDLE;
	};
	class Semaphore
	{
	public:
		Semaphore()
		{
			vk::SemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.flags = vk::SemaphoreCreateFlags();
			m_Semaphore =RenderContext::GetDevice()->GetDevice().createSemaphore(semaphoreInfo);
		};
		Semaphore(const Semaphore& fence) = delete;
		Semaphore(Semaphore&& semaphore)
		{
			m_Semaphore = semaphore.m_Semaphore;
			semaphore.m_Semaphore = VK_NULL_HANDLE;
		};

		~Semaphore()
		{
			Destroy();
		};
		void Destroy()
		{
			if (m_Semaphore != VK_NULL_HANDLE)
			{
				RenderContext::GetDevice()->GetDevice().waitIdle();
				RenderContext::GetDevice()->GetDevice().destroySemaphore(m_Semaphore);
				m_Semaphore = VK_NULL_HANDLE;
			}
		}
		vk::Semaphore GetSemaphore()
		{
			return m_Semaphore;
		}
	private:
		vk::Semaphore m_Semaphore = VK_NULL_HANDLE;
	};
	
}
