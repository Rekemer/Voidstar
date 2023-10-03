#pragma once
#include"vulkan/vulkan.hpp"
#include"../Types.h"
namespace Voidstar
{
	struct GraphicsPipelineSpecification
	{

		vk::Device device;
		std::string vertexFilepath = "";
		std::string fragmentFilepath = "";
		std::string geometryShader = "";

		// tesselation shaders
		// control shader
		std::string tessCFilepath = "";
		// evalatuation shader
		std::string tessEFilepath = "";


		std::string computeFilepath = "";



		vk::SampleCountFlagBits samples;
		vk::Extent2D swapchainExtent;
		vk::Format swapchainImageFormat;
		std::vector<vk::VertexInputBindingDescription> bindingDescription;
		std::vector<vk::VertexInputAttributeDescription>  attributeDescription;
		std::vector<vk::DescriptorSetLayout> descriptorSetLayout;
	};

	class Pipeline
	{
	public:
		static UPtr<Pipeline> CreateGraphicsPipeline(GraphicsPipelineSpecification& spec, vk::PrimitiveTopology topology, vk::Format depthFormat, vk::RenderPass renderpass, int subpass, bool writeToDepthBuffer, vk::PolygonMode polygonMode = vk::PolygonMode::eFill);
		static UPtr<Pipeline> CreateComputePipeline(std::string computeShader, std::vector<vk::DescriptorSetLayout>& layouts);
		vk::PipelineLayout GetLayout() const  { return m_PipelineLayout; }
		vk::Pipeline GetPipeline() const  { return m_Pipeline; }
		
		~Pipeline();

	private:

		friend class Renderer;
		vk::Pipeline m_Pipeline;
		vk::RenderPass m_RenderPass;
		vk::PipelineLayout m_PipelineLayout;
	};
}