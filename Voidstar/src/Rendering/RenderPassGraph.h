#pragma once
#include"RenderPass.h"
#include"unordered_map"
#include"string"
#include"functional"
#include"Sync.h"
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
		void AddRenderPass(UPtr<RenderPass> renderPass);
		~RenderPassGraph();
		void AddExec(std::string_view renderPassName );
		void Execute();
	private:
		std::unordered_map<std::string,UPtr<RenderPass>> m_RenderPasses;
		std::vector<RenderPass*> m_Transitions;
		std::vector<Fence> m_Fences;
		std::vector<Semaphore> m_SemaphoreImageAvailable;
		std::vector<Semaphore> m_SemaphoreRenderFinished;
		std::unordered_map<std::string, Pipeline> m_Pipelines;
	};
}
