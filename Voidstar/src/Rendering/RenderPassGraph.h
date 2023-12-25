#pragma once
#include"RenderPass.h"
#include"unordered_map"
#include"string"
#include"functional"
namespace Voidstar
{
	struct Transition
	{
		RenderPass* Next;
	};
	class RenderPassGraph;
	class CommandBuffer;
	class Pipeline;
	
	class RenderPassGraph
	{
	public:
		void AddRenderPass(UPtr<RenderPass> renderPass, std::initializer_list<Pipeline> pipeline);
		~RenderPassGraph();
	private:
		std::unordered_map<std::string_view, UPtr<RenderPass>> m_RenderPasses;
		std::unordered_map<RenderPass*, std::vector<Pipeline>> m_Pipelines;
		std::unordered_map<std::string, std::vector<Transition>> m_Transitions;
		std::unordered_map<std::string, Attachment> m_Transitions;
	};
}
