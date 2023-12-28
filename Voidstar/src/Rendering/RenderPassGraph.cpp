#include"Prereq.h"
#include "RenderPassGraph.h"
#include "Renderer.h"
namespace Voidstar
{
	void RenderPassGraph::Destroy()
	{
		for (auto& e : m_RenderPasses)
		{
			e.second.reset();
		}
		m_Semaphores.clear();
		m_Fence.~Fence();
	}
	void RenderPassGraph::AddRenderPass(UPtr<RenderPass> renderPass)
	{

		m_RenderPasses[renderPass->GetName().data()] = std::move(renderPass);
	}
	void RenderPassGraph::AddExec(std::string_view from)
	{
		Semaphore semaphore;
		m_Semaphores.push_back(std::move(semaphore));
		m_Transitions.push_back(m_RenderPasses.at(from.data()).get());
	}
	vk::Semaphore RenderPassGraph::Execute(CommandBuffer& cmd, size_t frameIndex, Semaphore& imageIsAvailable)
	{
		for (int i = 0; i < m_Transitions.size() ; i++)
		{
			Renderer::Instance()->Wait(m_Fence.GetFence());
			Renderer::Instance()->Reset(m_Fence.GetFence());
			m_Transitions[i]->Execute(cmd, frameIndex);
			
			if (i == 0)
			{
			cmd.Submit(&imageIsAvailable.GetSemaphore(),
				&m_Semaphores[i].GetSemaphore(), &m_Fence.GetFence());
			}
			else
			{
				cmd.Submit(&m_Semaphores[i-1].GetSemaphore(),
					&m_Semaphores[i ].GetSemaphore(), &m_Fence.GetFence());
			}
		}
		return m_Semaphores[m_Semaphores.size() - 1 ].GetSemaphore();
	}
}