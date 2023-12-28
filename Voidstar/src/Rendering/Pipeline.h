#pragma once
#include"vulkan/vulkan.hpp"
#include"../Types.h"
namespace Voidstar
{

	class Pipeline
	{
	public:
		static UPtr<Pipeline> CreateComputePipeline(std::string_view computeShader, std::vector<vk::DescriptorSetLayout>& layouts);
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
		void StencilTestOp(vk::CompareOp op, vk::StencilOp fail, vk::StencilOp pass, vk::StencilOp depthFailOp);
		void SetSamples(vk::SampleCountFlagBits samples);
		void SetRenderPass(vk::RenderPass renderPass);
		void SetSubpassAmount(int amount);
		void SetBlendOp();
		void SetStencilRefNumber(uint32_t number)
		{
			m_StencilRefNumber = number;
		}
		void SetDepthTest(bool test)
		{
			m_DepthTest = test;
		}
		void SetMasks(uint32_t compare, uint32_t write )
		{
			m_WriteMask = write;
			m_CompareMask = compare;
		}
		void SetBlendOp(vk::BlendOp op, vk::BlendFactor src, vk::BlendFactor dst)
		{
			m_BlendOp = op;
			m_BlendSrc = src;
			m_BlendDst = dst;
		}
		void SetSampleShading(vk::Bool32 state)
		{
			m_SampleShadingEnable = state; // enable sample shading in the pipeline
		}
		void DestroyShaderModules();
		void Build(std::string_view name);
		~PipelineBuilder();
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
		bool m_DepthTest = false; 
		uint32_t m_WriteMask = 0xff;
		uint32_t m_CompareMask = 0xff;
		vk::CompareOp  m_StencilOp = vk::CompareOp::eLess;
		vk::StencilOp m_StencilFailOp = vk::StencilOp::eReplace;
		vk::StencilOp m_StencilPassOp = vk::StencilOp::eReplace;
		vk::StencilOp m_DepthFailOp = vk::StencilOp::eReplace;
		uint32_t m_StencilRefNumber = 0;
		vk::RenderPass m_RenderPass;
		int m_SubpassNumber;

		vk::Bool32 m_BlendEnable = VK_TRUE;
		vk::Bool32 m_SampleShadingEnable = VK_FALSE;
		float m_MinSampleShading = .2f;
		vk::BlendOp m_BlendOp = vk::BlendOp::eAdd;
		vk::BlendFactor m_BlendSrc = vk::BlendFactor::eSrcAlpha;
		vk::BlendFactor m_BlendDst = vk::BlendFactor::eOneMinusSrcAlpha;

	};
}