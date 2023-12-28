#pragma once
#include "Prereq.h"
#include "RenderPass.h"
#include <unordered_map>
#include <string>
#include "functional"
#include "Sync.h"
namespace Voidstar
{
	struct Transition
	{
		
		RenderPass* Current;
		Transition* Next;
	};
	class RenderPassGraph;
	class CommandBuffer;
	class Pipeline;
	
	class RenderPassGraph
	{
	public:
		RenderPassGraph() = default;
		RenderPassGraph(const RenderPassGraph& ) = delete;
		RenderPassGraph& operator=(const RenderPassGraph& ) = delete;
		void AddRenderPass(UPtr<RenderPass> renderPass);
		void AddExec(std::string_view renderPassName );
		vk::Semaphore  Execute(CommandBuffer& cmd, size_t frameIndex, Semaphore& imageIsAvailable);
		void ExecuteOffline();
		void Destroy();
	private:
		std::unordered_map<std::string,UPtr<RenderPass>> m_RenderPasses;
		std::vector<RenderPass*> m_Transitions;
		Fence m_Fence;
		std::vector<Semaphore> m_Semaphores;
	};
}
