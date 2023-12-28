#include"Prereq.h"
#include "RenderPassGraph.h"
namespace Voidstar
{
	void RenderPassGraph::Destroy()
	{
		for (auto& e : m_RenderPasses)
		{
			e.second.reset();
		}
	}
	void RenderPassGraph::AddRenderPass(UPtr<RenderPass> renderPass)
	{

		m_RenderPasses[renderPass->GetName().data()] = std::move(renderPass);
	}
	void RenderPassGraph::AddExec(std::string_view from)
	{
		Semaphore semaphore;
		m_SemaphoreImageAvailable.push_back(std::move(semaphore));
		m_Transitions.push_back(m_RenderPasses.at(from.data()).get());
	}
	void RenderPassGraph::Execute(CommandBuffer& cmd, size_t frameIndex)
	{
		for (auto renderPass : m_Transitions)
		{
			renderPass->Execute(cmd,frameIndex);
		}
	}
}