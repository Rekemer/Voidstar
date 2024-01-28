#pragma once
#include "Prereq.h"
#include "RenderPass.h"
#include <unordered_map>
#include <string>
#include "functional"
#include "Sync.h"
namespace Voidstar
{
	struct IExecute;
	class RenderPassGraph;
	class CommandBuffer;
	class Pipeline;
	
	class RenderPassGraph
	{
	public:
		RenderPassGraph() = default;
		RenderPassGraph(const RenderPassGraph& ) = delete;
		RenderPassGraph& operator=(const RenderPassGraph& ) = delete;
		void AddExec(UPtr<IExecute> pass);
		vk::Semaphore  Execute(CommandBuffer& cmd, size_t frameIndex, Semaphore& imageIsAvailable);
		void Destroy();
	private:
		std::vector<UPtr<IExecute>> m_Transitions;
		Fence m_Fence;
		std::vector<Semaphore> m_Semaphores;
	};
}
