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
		Fence(Fence&& fence) = default;
		~Fence()
		{
			Destroy();
		}
		void Destroy()
		{
			RenderContext::GetDevice()->GetDevice().destroyFence(m_Fence);
		}
		vk::Fence GetFence()
		{
			return m_Fence;
		}
	private:
		vk::Fence m_Fence;
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
		Semaphore(Semaphore&& semaphore) = default;

		~Semaphore()
		{
			Destroy();
		};
		void Destroy()
		{
			RenderContext::GetDevice()->GetDevice().destroySemaphore(m_Semaphore);
		}
		vk::Semaphore GetSemaphore()
		{
			return m_Semaphore;
		}
	private:
		vk::Semaphore m_Semaphore;
	};
	
}
