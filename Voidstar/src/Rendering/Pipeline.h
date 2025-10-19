#pragma once
#include"vulkan/vulkan.hpp"
#include"../Types.h"
namespace Voidstar
{

	class VOIDSTAR_API Pipeline
	{
	public:
		static void CreateComputePipeline(std::string_view pipelineName,std::string_view computeShader, const std::vector<vk::DescriptorSetLayout>& layouts);
		vk::Pipeline GetPipeline() const  { return m_Pipeline; }
		Pipeline(const Pipeline& pipeline) = delete;
		Pipeline() = default;
		~Pipeline();

	private:

		friend class Renderer;
		friend class PipelineBuilder;
		vk::Pipeline m_Pipeline;
		vk::PipelineLayout m_PipelineLayout;
	};
	
}