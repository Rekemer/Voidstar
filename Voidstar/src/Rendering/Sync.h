#pragma once
#include"vulkan/vulkan.hpp"
#include"RenderContext.h"
#include"Device.h"
namespace Voidstar
{
	class VOIDSTAR_API Fence
	{
	public:
		Fence()
		{
			vk::FenceCreateInfo fenceInfo = {};
			fenceInfo.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;
			m_Fence = RenderContext::GetDevice()->GetDevice().createFence(fenceInfo);
		};
		Fence(const Fence& fence) = delete;
		void operator= (Fence&& fence)
		{
			m_Fence = fence.m_Fence;
			fence.m_Fence = VK_NULL_HANDLE;
		}
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
			if (m_Fence.operator!=(VK_NULL_HANDLE))
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
	class VOIDSTAR_API Semaphore
	{
	public:

		static std::vector<Semaphore> CreateBinarySemaphore(size_t amount)
		{
			std::vector<Semaphore> sem{};
			vk::SemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.flags = vk::SemaphoreCreateFlags();
			for (int i = 0; i < amount; i++)
			{
				Semaphore s;
				s.m_Semaphore = RenderContext::GetDevice()->GetDevice().createSemaphore(semaphoreInfo);
				sem.push_back(std::move(s));
			}
			return sem;
		};

		static std::vector<Semaphore> CreateTimelineSemaphore(size_t amount, size_t initValue)
		{
			vk::SemaphoreTypeCreateInfo typeInfo = { vk::SemaphoreType::eTimeline , initValue};
			std::vector<Semaphore> sem{};
			vk::SemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.flags = vk::SemaphoreCreateFlags();
			semaphoreInfo.sType = vk::StructureType::eSemaphoreCreateInfo;
			semaphoreInfo.pNext = static_cast<void*>(&typeInfo);
			for (int i = 0; i < amount; i++)
			{
				Semaphore s;
				s.m_Semaphore = RenderContext::GetDevice()->GetDevice().createSemaphore(semaphoreInfo);
				sem.push_back(std::move(s));
			}
			return sem;
		};
	
		Semaphore() = default;
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
			if (m_Semaphore.operator!=(VK_NULL_HANDLE))
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
