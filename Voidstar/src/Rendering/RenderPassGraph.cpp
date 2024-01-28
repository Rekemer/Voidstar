#include"Prereq.h"
#include "RenderPassGraph.h"
#include "Renderer.h"
namespace Voidstar
{
	void RenderPassGraph::Destroy()
	{
		for (auto& e : m_Transitions)
		{
			e.reset();
		}
		
		m_Semaphores.clear();
		m_Fence.~Fence();
	}
	void RenderPassGraph::AddExec(UPtr<IExecute> pass)
	{
		Semaphore semaphore;
		m_Semaphores.push_back(std::move(semaphore));
		m_Transitions.push_back(std::move(pass));
	}
	vk::Semaphore RenderPassGraph::Execute(CommandBuffer& cmd, size_t frameIndex, Semaphore& imageIsAvailable)
	{
		for (int i = 0; i < m_Transitions.size() ; i++)
		{
			Renderer::Instance()->Wait(m_Fence.GetFence());
			Renderer::Instance()->Reset(m_Fence.GetFence());
			if (m_Transitions[i]->IsCompute())
			{
				m_Transitions[i]->Execute(Renderer::Instance()->GetComputeCommandBuffer(frameIndex), frameIndex);
			}
			else m_Transitions[i]->Execute(cmd, frameIndex);
			
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
		return m_Semaphores[m_Transitions.size()-1].GetSemaphore();
	}
}