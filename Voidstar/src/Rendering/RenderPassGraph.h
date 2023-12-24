#pragma once
#include"RenderPass.h"
#include"unordered_map"
#include"string"
#include"functional"
namespace Voidstar
{
	class RenderPassGraph;
	class CommandBuffer;
	class Pipeline;
	using ExecFunc = std::function<void* (CommandBuffer& cmdBuffer)>;
	class RenderPassGraph
	{
	public:
		void AddRenderPass(RenderPass& renderPass, std::initializer_list<Pipeline> pipeline);
	private:
		std::unordered_map<std::string, RenderPass> m_RenderPasses;
		std::unordered_map<std::string, std::vector<RenderPass>> m_;
	};
}
