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
		friend class PipelineBuilder;
		vk::Pipeline m_Pipeline;
		vk::PipelineLayout m_PipelineLayout;
	};
	class PipelineBuilder
	{
	public:
		void SetDevice(vk::Device);
		void AddShader(std::string_view path, vk::ShaderStageFlagBits type);
		void AddBindingDescription(std::vector<vk::VertexInputBindingDescription>& bindings);
		void AddAttributeDescription(std::vector<vk::VertexInputAttributeDescription>& attributes);
		void AddDescriptorLayouts(std::vector<vk::DescriptorSetLayout>& layouts);
		void AddExtent(vk::Extent2D);
		void AddImageFormat(vk::Format);
		void SetTopology(vk::PrimitiveTopology topology);
		void SetPolygoneMode(vk::PolygonMode polygon);
		void AddPipelineLayout(vk::PipelineLayout layout);
		void SetControlPoints(int amountPoints);
		void WriteToDepthBuffer(bool wrtite);
		void EnableStencilTest(bool test);
		void StencilTestOp(vk::CompareOp op);
		void SetSamples(vk::SampleCountFlagBits samples);
		void SetRenderPass(vk::RenderPass renderPass);
		void SetSubpassAmount(int amount);
		void SetStencilRefNumber(uint32_t number)
		{
			m_StencilRefNumber = number;
		}
		UPtr<Pipeline> Build();
	private:
		std::vector<vk::ShaderModule> m_Modules;
		vk::PrimitiveTopology m_Topology;
		vk::PolygonMode m_PolygonMode;
		vk::Device m_Device;
		vk::PipelineLayout m_PipelineLayout;
		std::vector<vk::PipelineShaderStageCreateInfo> m_ShaderStages;
		std::vector<vk::VertexInputBindingDescription> m_Bindings;
		std::vector<vk::VertexInputAttributeDescription> m_Attributes;
		std::vector<vk::DescriptorSetLayout> m_DescriptorSetLayouts;
		vk::Format m_SwapchainImageFormat;
		vk::Extent2D m_Extent;
		vk::SampleCountFlagBits m_Samples;
		int m_PatchControlPoints = -1;
		bool m_WriteToDepthBuffer = false;
		bool m_StencilTest = false;
		vk::CompareOp  m_StencilOp = vk::CompareOp::eLess;
		 uint32_t m_StencilRefNumber = 0;
		vk::RenderPass m_RenderPass;
		int m_SubpassNumber;

	};
}