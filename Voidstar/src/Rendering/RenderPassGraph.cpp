#include"Prereq.h"
#include "RenderPassGraph.h"
namespace Voidstar
{
	void RenderPassGraph::Destroy()
	{

		for (auto&& e : m_RenderPasses)
		{
			e.second->Destroy();
		}
	}
	void RenderPassGraph::AddRenderPass(UPtr<RenderPass> renderPass)
	{

		auto ptr = std::move(renderPass);
		m_RenderPasses[renderPass->GetName().data()] = std::move(renderPass);
	}
	void RenderPassGraph::AddExec(std::string_view from)
	{
		//m_Transitions.push_back(m_RenderPasses.at(from.data()).get());
	}
}