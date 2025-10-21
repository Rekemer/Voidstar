
#include"Prereq.h"
#include "Renderer.h"
#include <set>

#include "../Window.h"
#include "../Log.h"
#include "glfw3.h"
#include "Instance.h"
#include "Device.h"
#include "Swapchain.h"
#include "SupportStruct.h"

#include "Buffer.h"
#include "IndexBuffer.h"
#include "RenderContext.h"
#include "DescriptorSetLayout.h"
#include "Camera.h"
#include "Image.h"
#include "DescriptorPool.h"
#include "Model.h"
#include "../Application.h"
#include <random>
#include "CommandPoolManager.h"

#include <filesystem>
#include <cstdlib>
#include <algorithm>

#include "Pipeline.h"
#include <random>
#include "Initializers.h"
#include "input.h"
#include  "Binder.h"
#include  "Generation.h"
#include"Settings.h"
#include"Sync.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>
#include <fstream>




namespace std
{
	template<>
	struct hash<std::pair<int,int>>
	{
		size_t operator()(const std::pair<int, int>& key) const
		{
			std::string keyString = std::to_string(key.first) + std::to_string(key.second);
			return std::hash<std::string>()(keyString);
		}
	};

}



namespace Voidstar
{
	class  PipelineBuilder
	{
	public:
		void AddShader(vk::ShaderModule module, vk::ShaderStageFlagBits type);
		void AddBindingDescription(const vk::VertexInputBindingDescription& bindings);
		void AddBindingDescription(const std::vector<vk::VertexInputBindingDescription>& bindings);
		void AddAttributeDescription(const std::vector<vk::VertexInputAttributeDescription>& attributes);
		void AddAttributeDescription(const vk::VertexInputAttributeDescription& attribute);
		void AddDescriptorSetLayouts(std::vector<vk::DescriptorSetLayout>& layouts);
		void AddExtent(vk::Extent2D);
		void SetTopology(vk::PrimitiveTopology topology);
		void SetPolygoneMode(vk::PolygonMode polygon);
		void AddPipelineLayout(vk::PipelineLayout layout);
		void SetControlPoints(int amountPoints);
		void WriteToDepthBuffer(bool wrtite);
		void EnableStencilTest(bool test);
		void StencilTestOp(vk::CompareOp op, vk::StencilOp fail, vk::StencilOp pass, vk::StencilOp depthFailOp);
		void SetSamples(vk::SampleCountFlagBits samples);
		void SetRenderPass(vk::RenderPass renderPass);
		void SetSubpassIndex(int amount);
		void EnableBlend(bool state)
		{
			m_BlendEnable = state;
		}
		void SetStencilRefNumber(uint32_t number)
		{
			m_StencilRefNumber = number;
		}
		void SetDepthTest(bool test)
		{
			m_DepthTest = test;
		}
		void SetMasks(uint32_t compare, uint32_t write)
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
		vk::Pipeline Build();
		~PipelineBuilder();
	private:
		std::vector<vk::ShaderModule> m_Modules;
		vk::PrimitiveTopology m_Topology;
		vk::PolygonMode m_PolygonMode;
		vk::PipelineLayout m_PipelineLayout;
		std::vector<vk::PipelineShaderStageCreateInfo> m_ShaderStages;
		std::vector<vk::VertexInputBindingDescription> m_Bindings;
		std::vector<vk::VertexInputAttributeDescription> m_Attributes;
		std::vector<vk::DescriptorSetLayout> m_DescriptorSetLayouts;
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
		int m_SubpassNumber = 0;

		vk::Bool32 m_BlendEnable = VK_TRUE;
		vk::Bool32 m_SampleShadingEnable = VK_FALSE;
		float m_MinSampleShading = .2f;
		vk::BlendOp m_BlendOp = vk::BlendOp::eAdd;
		vk::BlendFactor m_BlendSrc = vk::BlendFactor::eSrcAlpha;
		vk::BlendFactor m_BlendDst = vk::BlendFactor::eOneMinusSrcAlpha;

	};


	void PipelineBuilder::AddShader(vk::ShaderModule module, vk::ShaderStageFlagBits type)
	{
		m_Modules.push_back(module);
		{
			vk::PipelineShaderStageCreateInfo vertexShaderInfo = {};
			vertexShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
			vertexShaderInfo.stage = type;
			vertexShaderInfo.module = module;
			vertexShaderInfo.pName = "main";
			m_ShaderStages.push_back(vertexShaderInfo);
		}
	}


	void PipelineBuilder::AddBindingDescription(const vk::VertexInputBindingDescription& bindings)
	{
		m_Bindings.push_back(bindings);
	}
	void PipelineBuilder::AddBindingDescription(const std::vector<vk::VertexInputBindingDescription>& bindings)
	{
		m_Bindings.insert(m_Bindings.end(), bindings.begin(), bindings.end());
	}
	void PipelineBuilder::AddAttributeDescription(const std::vector<vk::VertexInputAttributeDescription>& attributes)
	{
		m_Attributes = attributes;
	}
	void PipelineBuilder::AddAttributeDescription(const vk::VertexInputAttributeDescription& attribute)
	{
		m_Attributes.push_back(attribute);
	}
	void PipelineBuilder::AddDescriptorSetLayouts(std::vector<vk::DescriptorSetLayout>& layouts)
	{
		m_DescriptorSetLayouts = layouts;
	}

	void PipelineBuilder::AddExtent(vk::Extent2D size)
	{
		m_Extent = size;
	}

	void PipelineBuilder::SetTopology(vk::PrimitiveTopology topology)
	{
		m_Topology = topology;
	}

	void PipelineBuilder::SetPolygoneMode(vk::PolygonMode polygon)
	{
		m_PolygonMode = polygon;
	}

	void PipelineBuilder::AddPipelineLayout(vk::PipelineLayout layout)
	{
		m_PipelineLayout = layout;
	}

	void PipelineBuilder::SetControlPoints(int amountPoints)
	{
		assert(amountPoints > 0);
		m_PatchControlPoints = amountPoints;
	}

	void PipelineBuilder::WriteToDepthBuffer(bool wrtite)
	{
		m_WriteToDepthBuffer = write;
	}

	void PipelineBuilder::EnableStencilTest(bool test)
	{
		m_StencilTest = test;
	}

	void PipelineBuilder::StencilTestOp(vk::CompareOp op, vk::StencilOp fail, vk::StencilOp pass, vk::StencilOp depthFailOp)
	{
		m_StencilOp = op;
		m_StencilFailOp = fail;
		m_StencilPassOp = pass;
		m_DepthFailOp = depthFailOp;
	}
	
	PipelineBuilder::~PipelineBuilder()
	{
	
	}
	void PipelineBuilder::SetSamples(vk::SampleCountFlagBits samples)
	{
		m_Samples = samples;
	}

	void PipelineBuilder::SetRenderPass(vk::RenderPass renderPass)
	{
		m_RenderPass = renderPass;
	}

	void PipelineBuilder::SetSubpassIndex(int numberOfSubpass)
	{
		m_SubpassNumber = numberOfSubpass;
	}

	vk::Pipeline PipelineBuilder::Build()
	{


		auto pipeline = CreateUPtr<Pipeline>();
		/*
		* Build and return a graphics pipeline based on the given info.
		*/

		//The info for the graphics pipeline
		vk::GraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.flags = vk::PipelineCreateFlags();


		vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};

		vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
		vertexInputInfo.vertexBindingDescriptionCount = m_Bindings.size();
		vertexInputInfo.pVertexBindingDescriptions = m_Bindings.data();

		vertexInputInfo.vertexAttributeDescriptionCount = m_Attributes.size();
		vertexInputInfo.pVertexAttributeDescriptions = m_Attributes.data();

		pipelineInfo.pVertexInputState = &vertexInputInfo;

		//Input Assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
		inputAssemblyInfo.topology = m_Topology;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

		vk::PipelineTessellationStateCreateInfo tesselationState;
		if (m_PatchControlPoints != -1)
		{
			tesselationState.patchControlPoints = m_PatchControlPoints;

			pipelineInfo.pTessellationState = &tesselationState;
		}









		//Viewport and Scissor
		vk::Viewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = m_Extent.width;
		viewport.height = m_Extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vk::Rect2D scissor = {};
		scissor.offset.x = 0.0f;
		scissor.offset.y = 0.0f;
		scissor.extent = m_Extent;
		vk::PipelineViewportStateCreateInfo viewportState = {};
		viewportState.flags = vk::PipelineViewportStateCreateFlags();
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
		pipelineInfo.pViewportState = &viewportState;

		const vk::DynamicState dynamicStates[] = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
		};
		vk::PipelineDynamicStateCreateInfo createInfo{};
		createInfo.pNext = nullptr;
		createInfo.flags = {};
		createInfo.dynamicStateCount = 2;
		createInfo.pDynamicStates = &dynamicStates[0];

		pipelineInfo.pDynamicState = &createInfo;


		//Rasterizer
		vk::PipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.flags = vk::PipelineRasterizationStateCreateFlags();
		rasterizer.depthClampEnable = VK_FALSE; //discard out of bounds fragments, don't clamp them
		rasterizer.rasterizerDiscardEnable = VK_FALSE; //This flag would disable fragment output
		rasterizer.polygonMode = m_PolygonMode;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = vk::CullModeFlagBits::eNone;
		rasterizer.frontFace = vk::FrontFace::eClockwise;
		rasterizer.depthBiasEnable = VK_FALSE; //Depth bias can be useful in shadow maps.
		pipelineInfo.pRasterizationState = &rasterizer;






		pipelineInfo.stageCount = m_ShaderStages.size();
		pipelineInfo.pStages = m_ShaderStages.data();




		vk::PipelineDepthStencilStateCreateInfo depthState;
		depthState.flags = vk::PipelineDepthStencilStateCreateFlags();
		depthState.depthTestEnable = m_DepthTest;
		depthState.depthWriteEnable = m_WriteToDepthBuffer;
		depthState.depthCompareOp = vk::CompareOp::eLess;
		depthState.depthBoundsTestEnable = false;
		depthState.stencilTestEnable = m_StencilTest;
		depthState.back.compareOp = m_StencilOp;
		depthState.back.failOp = m_StencilFailOp;
		depthState.back.passOp = m_StencilPassOp;
		depthState.back.compareMask = m_CompareMask;
		depthState.back.writeMask = m_WriteMask;
		depthState.back.reference = m_StencilRefNumber;
		depthState.front = depthState.back;

		pipelineInfo.pDepthStencilState = &depthState;

		//Multisampling
		vk::PipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.flags = vk::PipelineMultisampleStateCreateFlags();
		multisampling.sampleShadingEnable = m_SampleShadingEnable;
		multisampling.minSampleShading = m_MinSampleShading;
		multisampling.rasterizationSamples = m_Samples;
		pipelineInfo.pMultisampleState = &multisampling;

		//Color Blend
		vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;


		colorBlendAttachment.blendEnable = m_BlendEnable;
		colorBlendAttachment.colorBlendOp = m_BlendOp;
		colorBlendAttachment.srcColorBlendFactor = m_BlendSrc;
		colorBlendAttachment.dstColorBlendFactor = m_BlendDst;



		vk::PipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.flags = vk::PipelineColorBlendStateCreateFlags();
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = vk::LogicOp::eCopy;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;
		pipelineInfo.pColorBlendState = &colorBlending;

		pipelineInfo.layout = m_PipelineLayout;




		//Renderpass

		pipelineInfo.renderPass = m_RenderPass;
		pipelineInfo.subpass = m_SubpassNumber;


		//Extra stuff
		pipelineInfo.basePipelineHandle = nullptr;

		//Make the Pipeline

		vk::Pipeline graphicsPipeline;
		try
		{
			auto pipeline = RenderContext::GetDevice()->GetDevice().createGraphicsPipeline(nullptr, pipelineInfo).value;
			Log::GetLog()->info("Pipeline is Created!");
			return pipeline;
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to create Pipeline");
			return {};
		}

	}


}


namespace Voidstar
{


	static vk::BlendFactor map(BlendFactor f) {
		switch (f) {
		case BlendFactor::Zero:               return vk::BlendFactor::eZero;
		case BlendFactor::One:                return vk::BlendFactor::eOne;
		case BlendFactor::SrcColor:           return vk::BlendFactor::eSrcColor;
		case BlendFactor::OneMinusSrcColor:   return vk::BlendFactor::eOneMinusSrcColor;
		case BlendFactor::DstColor:           return vk::BlendFactor::eDstColor;
		case BlendFactor::OneMinusDstColor:   return vk::BlendFactor::eOneMinusDstColor;
		case BlendFactor::SrcAlpha:           return vk::BlendFactor::eSrcAlpha;
		case BlendFactor::OneMinusSrcAlpha:   return vk::BlendFactor::eOneMinusSrcAlpha;
		case BlendFactor::DstAlpha:           return vk::BlendFactor::eDstAlpha;
		case BlendFactor::OneMinusDstAlpha:   return vk::BlendFactor::eOneMinusDstAlpha;
		case BlendFactor::ConstColor:         return vk::BlendFactor::eConstantColor;
		case BlendFactor::OneMinusConstColor: return vk::BlendFactor::eOneMinusConstantColor;
		case BlendFactor::ConstAlpha:         return vk::BlendFactor::eConstantAlpha;
		case BlendFactor::OneMinusConstAlpha: return vk::BlendFactor::eOneMinusConstantAlpha;
		case BlendFactor::SrcAlphaSaturate:   return vk::BlendFactor::eSrcAlphaSaturate;
		}
		return vk::BlendFactor::eOne;
	}

	static vk::BlendOp map(BlendOp op) {
		switch (op) {
		case BlendOp::Add:             return vk::BlendOp::eAdd;
		case BlendOp::Subtract:        return vk::BlendOp::eSubtract;
		case BlendOp::ReverseSubtract: return vk::BlendOp::eReverseSubtract;
		case BlendOp::Min:             return vk::BlendOp::eMin;
		case BlendOp::Max:             return vk::BlendOp::eMax;
		}
		return vk::BlendOp::eAdd;
	}

	static vk::CompareOp map(CompareOp c) {
		switch (c) {
		case CompareOp::Never:    return vk::CompareOp::eNever;
		case CompareOp::Less:     return vk::CompareOp::eLess;
		case CompareOp::Equal:    return vk::CompareOp::eEqual;
		case CompareOp::LessEqual:   return vk::CompareOp::eLessOrEqual;
		case CompareOp::Greater:  return vk::CompareOp::eGreater;
		case CompareOp::NotEqual: return vk::CompareOp::eNotEqual;
		case CompareOp::GreaterEqual:   return vk::CompareOp::eGreaterOrEqual;
		case CompareOp::Always:   return vk::CompareOp::eAlways;
		}
		return vk::CompareOp::eLessOrEqual;
	}

	static vk::CullModeFlags map(Culling c) {
		switch (c) {
		case Culling::None:  return vk::CullModeFlagBits::eNone;
		case Culling::Back:  return vk::CullModeFlagBits::eBack;
		case Culling::Front: return vk::CullModeFlagBits::eFront;
		}
		return vk::CullModeFlagBits::eBack;
	}

	static vk::PolygonMode map(Polygon p) {
		switch (p) {
		case Polygon::Fill:  return vk::PolygonMode::eFill;
		case Polygon::Line:  return vk::PolygonMode::eLine;
		case Polygon::Point: return vk::PolygonMode::ePoint;
		}
		return vk::PolygonMode::eFill;
	}

	static vk::PrimitiveTopology map(Topology t) {
		switch (t) {
		case Topology::TriList:   return vk::PrimitiveTopology::eTriangleList;
		case Topology::TriStrip:  return vk::PrimitiveTopology::eTriangleStrip;
		case Topology::LineList:  return vk::PrimitiveTopology::eLineList;
		case Topology::LineStrip: return vk::PrimitiveTopology::eLineStrip;
		case Topology::Point:     return vk::PrimitiveTopology::ePointList;
		}
		return vk::PrimitiveTopology::eTriangleList;
	}

	static vk::StencilOp map(StencilOp o) {
		switch (o) {
		case StencilOp::Keep:      return vk::StencilOp::eKeep;
		case StencilOp::Zero:      return vk::StencilOp::eZero;
		case StencilOp::Replace:   return vk::StencilOp::eReplace;
		case StencilOp::IncrClamp: return vk::StencilOp::eIncrementAndClamp;
		case StencilOp::DecrClamp: return vk::StencilOp::eDecrementAndClamp;
		case StencilOp::Invert:    return vk::StencilOp::eInvert;
		case StencilOp::IncrWrap:  return vk::StencilOp::eIncrementAndWrap;
		case StencilOp::DecrWrap:  return vk::StencilOp::eDecrementAndWrap;
		}
		return vk::StencilOp::eKeep;
	}

	static vk::Format map(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::FLOAT:  return vk::Format::eR32Sfloat;
		case ShaderDataType::FLOAT2: return vk::Format::eR32G32Sfloat;
		case ShaderDataType::FLOAT3: return vk::Format::eR32G32B32Sfloat;
		case ShaderDataType::FLOAT4: return vk::Format::eR32G32B32A32Sfloat;
		default:
			throw std::runtime_error("Unknown ShaderDataType");
		}
	}




	QuadData quad;
	std::vector<Vertex_> sphere;
	std::vector<IndexType> sphereIndicies;
	
	const int QUAD_AMOUNT = 700;

	
	void Renderer::CreateLayouts()
	{
		for (auto [key, value] : m_Bindings)
		{
			m_Layout[key] = DescriptorSetLayout::Create(value);
		}
	}
	std::vector<vk::DescriptorSet> Renderer::AllocateSets(size_t amount, const DescriptorLayoutKey& key)
	{
		if (m_DescriptorSet.find(key) == m_DescriptorSet.end())
		{
			auto& layout = m_DescriptorLayout.at(key);
			std::vector<vk::DescriptorSetLayout> layouts{amount, layout};
			m_DescriptorSet[key] = m_UniversalPool->AllocateDescriptorSets(amount, layouts.data());
		}
		return  m_DescriptorSet[key];
	}
	void Renderer::CleanUpLayouts()
	{
		auto device = RenderContext::GetDevice();
		for (auto [key, value] : m_Layout)
		{
			device->GetDevice().destroyDescriptorSetLayout(value->GetLayout());
		}
	}


	CommandBuffer& Renderer::GetRenderCommandBuffer(size_t frameindex)
	{
		assert(frameindex < m_RenderCommandBuffer.size());
		return m_RenderCommandBuffer[frameindex];
	}

	CommandBuffer& Renderer::GetComputeCommandBuffer(size_t frameindex)
	{
		assert(frameindex < m_ComputeCommandBuffer.size());
		return m_ComputeCommandBuffer[frameindex];
	}

	CommandBuffer& Renderer::GetTransferCommandBuffer(size_t frameindex)
	{
		assert(frameindex < m_TransferCommandBuffer.size());
		return m_TransferCommandBuffer[frameindex];
	}

	void Renderer::BeginBatch()
	{
		m_QuadIndex = 0;
		m_BatchQuad = m_BatchQuadStart;
		//m_BatchInstance = m_BatchInstanceStart;
	}

	void Renderer::DrawBatch(vk::CommandBuffer& commandBuffer,size_t offset, int index)
	{
		vk::DeviceSize offsets[] = { offset };

		{
			vk::Buffer vertexBuffers[] = { m_QuadBufferBatch->GetBuffer() };
			commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		}
		commandBuffer.bindIndexBuffer(m_QuadBufferBatchIndex->GetBuffer(), 0, m_QuadBufferBatchIndex->GetIndexType());
		commandBuffer.drawIndexed(m_QuadIndex, 1, 0, 0, 0);
	}
	void Renderer::DrawBatchCustom(vk::CommandBuffer& commandBuffer, size_t indexAmount,size_t offset, int index)
	{
		vk::DeviceSize offsets[] = { offset };

		{
			vk::Buffer vertexBuffers[] = { m_QuadBufferBatch->GetBuffer() };
			commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		}
		commandBuffer.bindIndexBuffer(m_QuadBufferBatchIndex->GetBuffer(), 0, m_QuadBufferBatchIndex->GetIndexType());
		commandBuffer.drawIndexed(indexAmount, 1, m_QuadIndex-index, 0, 0);
	}

	void UpdateVertex(Vertex_*& vertex, glm::vec3 position, glm::vec2 uv,glm::vec4& color, glm::mat4& world,  int vertIndex)
	{
		vertex->Position = world * glm::vec4{ position,1 };
		vertex->UV = uv;
		vertex->Color = color;
	};
	void UpdateVerticies(Vertex_*& vertex, std::vector<Vertex_>& verticies)
	{
		assert(false);
		//UpdateVertex(vertex,verticies[0].Position, verticies[0].Color, glm::identity<glm::mat4>());
		//vertex++;																				   
		//UpdateVertex(vertex,verticies[2].Position, verticies[2].Color, glm::identity<glm::mat4>());
		//vertex++;																				   
		//UpdateVertex(vertex,verticies[3].Position, verticies[3].Color, glm::identity<glm::mat4>());
		//vertex++;																				   
		//UpdateVertex(vertex,verticies[1].Position, verticies[1].Color, glm::identity<glm::mat4>());
		//vertex++;

	};
	void Renderer::DrawTxt(vk::CommandBuffer commandBuffer, std::string_view str, glm::vec2 pos, std::map< unsigned char, Character>& characters)
	{
		float scale = 1;
		float scaleX = 1;
		auto offset = pos;
		for (auto e : str)
		{

			if (e == '\n')
			{
				offset.x = pos.x;
				pos.y -= 3* CharacterLineSpacing /64.f;
				continue;
			}
			else if (e == ' ')
			{
				offset.x += 5;
				continue;

			}
			else if (e == '\t')
			{
				offset.x += 30;
				continue;

			}
			if (characters.find(e) == characters.end()) continue;
		auto& characterData = characters.at(e);
		offset.x = offset.x + characterData.Bearing.x* scale;
		// to account for letter like p and q
		offset.y = pos.y - ( characterData.Size.y - characterData.Bearing.y)* scale;
		glm::vec4 color{ 1 };
		glm::mat4 world{ 1 };
		// left bottom
		m_BatchQuad->Position = glm::vec3{ offset.x ,offset.y,0};
		m_BatchQuad->UV = { characterData.minUv.x,characterData.maxUv.y };
		
		m_BatchQuad++;
		// right bottom
		m_BatchQuad->Position = glm::vec3{ offset.x + characterData.Size.x * scaleX ,offset.y,0 };
		m_BatchQuad->UV = { characterData.maxUv.x,characterData.maxUv.y };
		m_BatchQuad++;
		// right top
		m_BatchQuad->Position = glm::vec3{ offset.x + characterData.Size.x * scaleX,offset.y + characterData.Size.y * scale,0 };
		m_BatchQuad->UV = { characterData.maxUv.x,characterData.minUv.y };
		m_BatchQuad++;
		
		
		// left top
		m_BatchQuad->Position = glm::vec3{ offset.x ,offset.y + characterData.Size.y * scale,0 };
		m_BatchQuad->UV = { characterData.minUv.x,characterData.minUv.y };
		m_BatchQuad++;
		
		
		offset.x += characterData.Advance / 64.f* scaleX;
		m_QuadIndex += 6;
		}
	}
	void Renderer::DrawQuadScreen(vk::CommandBuffer commandBuffer)
	{
		//vk::DeviceSize offsets[] = { 0 };
		//{
		//	vk::Buffer vertexBuffers[] = { m_QuadBufferBatch->GetBuffer() };
		//	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		//
		//}
		//commandBuffer.bindIndexBuffer(m_QuadBufferBatchIndex->GetBuffer(), 0, m_QuadBufferBatchIndex->GetIndexType());
		commandBuffer.draw(6, 1, 0, 0);
	}

	void Renderer::Draw(Sphere& sphere)
	{
		DrawSphere(sphere.Pos,sphere.Scale,sphere.Color,sphere.Rot);
	}
	
	void Renderer::Draw(Quad& quad, glm::mat4& world)
	{
		DrawQuad(world, glm::vec4{1,0,1,1});
	}

	void Renderer::Draw(QuadRangle& quadrangle)
	{
		DrawQuad(quadrangle.Verticies);
	}
	void Renderer::DrawSphere(glm::vec3 pos, glm::vec3 scale, glm::vec4 color, glm::vec3 rot)
	{
		assert(false);
		//m_BatchInstance->Color = color;
		auto iden = glm::identity<glm::mat4>();
		iden = glm::translate(iden, pos);
		auto rotMatrix =  glm::rotate(iden, glm::radians(rot.x), glm::vec3{ 1,0,0 });
		rotMatrix = glm::rotate(rotMatrix, glm::radians(rot.y), glm::vec3{ 0,1,0 });
		rotMatrix = glm::scale(rotMatrix,scale);
		auto transpose = glm::transpose(rotMatrix);
		//m_BatchInstance->WorldMatrix= transpose;
		//m_BatchInstance++;
	}

	void Renderer::DrawSphereInstance(vk::CommandBuffer& commandBuffer)
	{
		assert(false);
		//vk::DeviceSize offsets[] = { 0 };
		//
		//{
		//	vk::Buffer vertexBuffers[] = { m_SphereBuffer->GetBuffer() };
		//	vk::Buffer instanceBuffers[] = { m_InstanceBuffer->GetBuffer() };
		//	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		//	commandBuffer.bindVertexBuffers(1, 1, instanceBuffers, offsets);
		//
		//}
		//commandBuffer.bindIndexBuffer(m_SphereIndexBuffer->GetBuffer(), 0, m_SphereIndexBuffer->GetIndexType());
		////auto instanceAmount = static_cast<uint64_t>(m_BatchInstance - m_BatchInstanceStart);
		//auto instanceAmount = static_cast<uint64_t>(0);
		//commandBuffer.drawIndexed(m_SphereIndexBuffer->GetIndexAmount(), instanceAmount, 0, 0, 0);
	}
	void Renderer::DrawQuad(glm::mat4& world, glm::vec4 color)
	{
		assert(false);
		//auto& verticies = quad.verticies;
		//// left bottom
		//UpdateVertex(m_BatchQuad,verticies[0].Position,color,world,0);
		//m_BatchQuad++;
		//// right bottom
		//UpdateVertex(m_BatchQuad,verticies[2].Position,color,world,2);
		//m_BatchQuad++;
		//// right top
		//UpdateVertex(m_BatchQuad, verticies[3].Position, color, world);
		//m_BatchQuad++;


		//// left top
		//UpdateVertex(m_BatchQuad, verticies[1].Position, color, world);
		//m_BatchQuad++;

		//m_QuadIndex += 6;
	}

	void Renderer::DrawQuad(std::vector<Vertex_>& verticies)
	{
		assert(false);
		//UpdateVerticies(m_BatchQuad, verticies);
		//m_QuadIndex += 6;
	}
	



	
	void Renderer::Init(size_t screenWidth, size_t screenHeight, std::shared_ptr<Window> window, Application* app) 
		
	{
		m_App = app;
		m_Window=window; 
		m_ViewportWidth = screenWidth;  
		m_ViewportHeight = screenHeight;
		m_CommandPoolManager = CreateUPtr<CommandPoolManager>();
		// create instance
		CreateInstance();


		RenderContext::CreateSurface(window.get());
		RenderContext::CreateDevice();

		m_Fence = std::move(Fence::Create());
		m_Device = RenderContext::GetDevice();

		RenderContext::CreateSwapchain(vk::Format::eB8G8R8A8Unorm,
			m_ViewportWidth, m_ViewportHeight,
			vk::PresentModeKHR::eFifo, vk::ColorSpaceKHR::eSrgbNonlinear) ;
		


		std::vector<vk::DescriptorPoolSize> pool_sizes =
		{
			{ vk::DescriptorType::eCombinedImageSampler, 10 },
			{ vk::DescriptorType::eStorageImage, 10 },
			{ vk::DescriptorType::eStorageBuffer, 10 },
			{ vk::DescriptorType::eInputAttachment, 10 },
			{ vk::DescriptorType::eUniformBuffer, 10 },
		};

		m_UniversalPool = DescriptorPool::Create(pool_sizes, 10);

	
		auto frameAmount = RenderContext::GetFrameAmount();
		m_FrameCommandPool = Renderer::Instance()->GetCommandPoolManager()->GetFreePool();
		m_RenderCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, frameAmount);
		m_TransferCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, frameAmount);
		m_ComputeCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, frameAmount);
		
		


		// get frame amount
		auto framesAmount = RenderContext::GetFrameAmount();
		auto m_Device = RenderContext::GetDevice();
		auto bufferSize = sizeof(UniformBufferObject);
		m_UniformBuffers.resize(framesAmount);
		m_UniformBuffersMapped.resize(framesAmount);


		BufferInputChunk inputBuffer;
		inputBuffer.size = bufferSize;
		inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		inputBuffer.usage = vk::BufferUsageFlagBits::eUniformBuffer;

		for (size_t i = 0; i < framesAmount; i++)
		{
			m_UniformBuffers[i] = CreateUPtr<Buffer>(inputBuffer);
			m_UniformBuffersMapped[i] = m_Device->GetDevice().mapMemory(m_UniformBuffers[i]->GetMemory(), 0, bufferSize);
		}



		SystemDescriptorLayoutKey.set = 0;
		BindingDesc desc;
		desc.binding = 0;
		desc.set = 0;
		desc.access = ShaderType::ALL;
		desc.kind = ResourceType::UniformBuffer;
		desc.count = 1;
		SystemDescriptorLayoutKey.bindings.insert(desc);
		
		CreateDescriptorLayout(SystemDescriptorLayoutKey);


		auto sets = AllocateSets(frameAmount, SystemDescriptorLayoutKey);

		for (auto i = 0; i < frameAmount; i++)
		{
			for (auto binding : SystemDescriptorLayoutKey.bindings)
			{
				m_Device->UpdateDescriptorSet(sets[i], binding.binding, binding.count, *m_UniformBuffers[i], binding.kind);
			}
		}


		m_AttachmentManager.Init(RenderContext::GetFrames());


		auto samples = RenderContext::GetDevice()->GetSamples();
		m_AttachmentManager.CreateColor("MSAA", m_AttachmentManager, vk::Format::eB8G8R8A8Unorm,
			screenWidth, screenHeight,
			samples, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
			frameAmount);

		m_AttachmentManager.CreateDepthStencil("DepthStencil", m_AttachmentManager,
			screenWidth, screenHeight, 
			samples, vk::ImageUsageFlagBits::eDepthStencilAttachment,
			frameAmount);


		DEFAULT_RENDER_PASS = g_RenderPassAllocator.GetId() ;
		

		RenderPassBuilder builder;

		builder.ColorOutput("MSAA", m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
		builder.SetLoadOp(vk::AttachmentLoadOp::eClear);
		builder.SetSaveOp(vk::AttachmentStoreOp::eDontCare);
		builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
		builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
		builder.SetInitialLayout(vk::ImageLayout::eUndefined);
		builder.SetFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

		builder.BuildAttachmentDesc();

		builder.DepthStencilOutput("DepthStencil", m_AttachmentManager, vk::ImageLayout::eDepthStencilAttachmentOptimal);
		builder.SetLoadOp(vk::AttachmentLoadOp::eClear);
		builder.SetSaveOp(vk::AttachmentStoreOp::eDontCare);
		builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
		builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
		builder.SetInitialLayout(vk::ImageLayout::eUndefined);
		builder.SetFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		builder.BuildAttachmentDesc();

		builder.ResolveOutput("Default", m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
		builder.SetLoadOp(vk::AttachmentLoadOp::eDontCare);
		builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
		builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
		builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
		builder.SetInitialLayout(vk::ImageLayout::eUndefined);
		//builder.SetFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
		builder.SetFinalLayout(vk::ImageLayout::ePresentSrcKHR);
		builder.BuildAttachmentDesc();

		vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
			vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);



		builder.AddSubpass({ 0 }, { 1 }, { 2 });

		builder.AddSubpassDependency(dependency0);

		

		vk::Extent2D extent = { static_cast<uint32_t>(screenWidth),static_cast<uint32_t>(screenHeight)};

		vk::ClearValue clearColor = { std::array<float, 4>{137.f / 255.f, 189.f / 255.f, 199.f / 255.f, 1.0f} };
		vk::ClearValue clearDepth = vk::ClearDepthStencilValue{ 1.0f, 0 };
		std::vector<vk::ClearValue> clearValues{ clearColor ,clearDepth, clearColor };
		m_RenderPasses[DEFAULT_RENDER_PASS] = builder.Build(m_AttachmentManager, RenderContext::GetFrameAmount(), extent, clearValues);





#if 0
		quad = GeneratePlane(1);
		sphere = GenerateSphere(1,10, sphereIndicies);

		auto& verticies = quad.verticies;
		auto& indices = quad.indicies;
		auto indexSize = SizeOfBuffer(indices.size(), indices[0]);
		{
			{
				SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(indexSize);


				{
					BufferInputChunk inputBuffer;
					inputBuffer.size = indexSize;
					inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
					inputBuffer.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
					m_QuadIndexBuffer = CreateUPtr<IndexBuffer>(inputBuffer, indices.size(), vk::IndexType::eUint32);

				}

				m_TransferCommandBuffer[0].BeginTransfering();
				m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_QuadIndexBuffer.get(), (void*)indices.data(), indexSize);
				m_TransferCommandBuffer[0].EndTransfering();
				m_TransferCommandBuffer[0].SubmitSingle();
			}
			{


				auto vertexSize = SizeOfBuffer(verticies.size(), verticies[0]);
				{
					BufferInputChunk inputBuffer;
					inputBuffer.size = vertexSize;
					inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
					inputBuffer.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;

					m_QuadBuffer = CreateUPtr<Buffer>(inputBuffer);


					
					{
						BufferInputChunk inputBuffer;
						inputBuffer.size = sizeof(Vertex) * 4 * QUAD_AMOUNT;
						inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
						inputBuffer.usage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eVertexBuffer;
						m_QuadBufferBatch = CreateUPtr<Buffer>(inputBuffer);
						m_BatchQuadStart = reinterpret_cast<Vertex*>( m_Device->GetDevice().mapMemory(m_QuadBufferBatch->GetMemory(),0, inputBuffer.size));

						indices.resize(QUAD_AMOUNT*3);
						int offset = 0;
						for (int i = 0; i < QUAD_AMOUNT*3; i+=6)
						{
							indices[i] = offset;
							indices[i + 1] = offset + 1;
							indices[i + 2] = offset + 2;

							indices[i + 3] = offset + 2;
							indices[i + 4] = offset + 3;
							indices[i + 5] = offset;

							offset += 4;
						}
						{
							BufferInputChunk inputBuffer;
							inputBuffer.size = SizeOfBuffer(indices.size(),indices[0]);
							inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
							inputBuffer.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
							m_QuadBufferBatchIndex = CreateUPtr<IndexBuffer>(inputBuffer, indices.size(), vk::IndexType::eUint32);


							auto indexSize = SizeOfBuffer(indices.size(), indices[0]);
							SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(indexSize);

							m_TransferCommandBuffer[0].BeginTransfering();
							m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_QuadBufferBatchIndex.get(), (void*)indices.data(), indexSize);
							m_TransferCommandBuffer[0].EndTransfering();
							m_TransferCommandBuffer[0].SubmitSingle();

						}
					}
				}
				void* vertexData = const_cast<void*>(static_cast<const void*>(verticies.data()));
				SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(vertexSize);
				m_TransferCommandBuffer[0].BeginTransfering();
				m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_QuadBuffer.get(), (void*)verticies.data(), vertexSize);
				m_TransferCommandBuffer[0].EndTransfering();
				m_TransferCommandBuffer[0].SubmitSingle();
			}



		}


		{
			auto indexSize = SizeOfBuffer(sphereIndicies.size(), sphereIndicies[0]);
			{
				{
					SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(indexSize);


					{
						BufferInputChunk inputBuffer;
						inputBuffer.size = indexSize;
						inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
						inputBuffer.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
						m_SphereIndexBuffer = CreateUPtr<IndexBuffer>(inputBuffer, sphereIndicies.size(), vk::IndexType::eUint32);

					}

					m_TransferCommandBuffer[0].BeginTransfering();
					m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_SphereIndexBuffer.get(), (void*)sphereIndicies.data(), indexSize);
					m_TransferCommandBuffer[0].EndTransfering();
					m_TransferCommandBuffer[0].SubmitSingle();
				}
				{
					auto vertexSize = SizeOfBuffer(sphere.size(), sphere[0]);
					{
						BufferInputChunk inputBuffer;
						inputBuffer.size = vertexSize;
						inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
						inputBuffer.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;

						m_SphereBuffer = CreateUPtr<Buffer>(inputBuffer);
					}
					SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(vertexSize);
					m_TransferCommandBuffer[0].BeginTransfering();
					m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_SphereBuffer.get(), (void*)sphere.data(), vertexSize);
					m_TransferCommandBuffer[0].EndTransfering();
					m_TransferCommandBuffer[0].SubmitSingle();
				}
			}
		}
		

		{
			BufferInputChunk inputBuffer;
			inputBuffer.size = sizeof(InstanceData) * 30;
			inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			inputBuffer.usage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eVertexBuffer;
			m_InstanceBuffer = CreateUPtr<Buffer>(inputBuffer);
			m_BatchInstanceStart = reinterpret_cast<InstanceData*>(m_Device->GetDevice().mapMemory(m_InstanceBuffer->GetMemory(), 0, inputBuffer.size));

		}

		
#endif // 0
			
	}

	
	

	

	



	


	
	void Renderer::Draw(Drawable& drawable)
	{
		drawable.m_Self->Draw();
	}
	void Renderer::UserInit()
	{
		CreateLayouts();
		//AllocateSets();

	}
	void Renderer::CreateSyncObjects()
	{	
		auto frameAmount = RenderContext::GetFrameAmount();
		m_ComputeInFlightFences.resize(frameAmount);
		m_ComputeFinishedSemaphores.resize(frameAmount);
		m_ImageAvailableSemaphore.resize(frameAmount);
		m_RenderFinishedSemaphore.resize(frameAmount);
		m_InFlightFence.resize(frameAmount);
	
	}




	void Renderer::RecreateSwapchain()
	{
		m_ViewportWidth = 0;
		m_ViewportHeight = 0;
		while (m_ViewportWidth == 0 || m_ViewportHeight == 0) {
			glfwGetFramebufferSize(m_Window->GetRaw(), &m_ViewportWidth, &m_ViewportHeight);
			glfwWaitEvents();
		}


		m_Device->GetDevice().waitIdle();


		SwapChainSupportDetails support;
		auto device = RenderContext::GetDevice();
		auto surface = RenderContext::GetSurface();
		support.AvailableCapabilities = device->GetDevicePhys().getSurfaceCapabilitiesKHR(*surface);
		support.AvailablePresentModes = device->GetDevicePhys().getSurfacePresentModesKHR(*surface);
		support.AvailableFormats = device->GetDevicePhys().getSurfaceFormatsKHR(*surface);
		support.ViewportWidth = m_ViewportWidth;
		support.ViewportHeight = m_ViewportHeight;
		
		RenderContext::RecreateSwapchain(support);
		
		//auto& camera = m_App->GetCamera();
		//camera->UpdateProj(m_ViewportWidth, m_ViewportHeight, camera->GetFov());
	}

	void Renderer::Shutdown()
	{

			
			


			auto device = m_Device->GetDevice();
			device.waitIdle();


				

				std::for_each(m_Graphs.begin(),
					m_Graphs.end(),
					[](UPtr<RenderPassGraph>& graph)
					{
						graph->Destroy();
					});

			for (int i = 0; i < m_ComputeCommandBuffer.size(); i++)
			{
				m_RenderCommandBuffer[i].Free();
				m_ComputeCommandBuffer[i].Free();
				m_TransferCommandBuffer[i].Free();
			};

			Renderer::Instance()->GetCommandPoolManager()->FreePool(m_FrameCommandPool);



			m_UniformBuffers.clear();
			m_UniversalPool.reset();
			CleanUpLayouts();
			m_ImageAvailableSemaphore.clear();
			m_RenderFinishedSemaphore.clear();
			m_ComputeFinishedSemaphores.clear();
			m_InFlightFence.clear();
			m_ComputeInFlightFences.clear();
			m_ImageAvailableSemaphore.clear(); 


			m_Pipelines.clear();
			m_CommandPoolManager->Release();
			m_InstanceBuffer.reset();
			m_QuadBufferBatch.reset();
			m_QuadBufferBatchIndex.reset();
			RenderContext::Shutdown();
			


		
	}


	

	
	Renderer* Renderer::Instance()
	{
		static Renderer* renderer = new Renderer;
		return renderer;
	}

	void Renderer::CompileShader(std::string_view path)
	{
		m_Compiler.Compile(std::filesystem::path{path});
	}
	void Renderer::LinkShaders(ProgramHandle handle ,uint8_t shaderAmount)
	{
		m_Compiler.Link(handle, shaderAmount);
	}


	vk::MemoryPropertyFlags GetMemoryFlags(UpdateHint hint)
	{
		switch (hint)	
		{
		case Voidstar::UpdateHint::Immutable:
		case Voidstar::UpdateHint::Static:
			return vk::MemoryPropertyFlagBits::eDeviceLocal;
			break;
		case Voidstar::UpdateHint::Dynamic:
			return vk::MemoryPropertyFlagBits::eHostCoherent |
				vk::MemoryPropertyFlagBits::eHostVisible;
			break;
		case Voidstar::UpdateHint::Readback:
			return vk::MemoryPropertyFlagBits::eHostCoherent |
				vk::MemoryPropertyFlagBits::eHostVisible |
				vk::MemoryPropertyFlagBits::eHostCached;
			break;
		default:
			return vk::MemoryPropertyFlagBits::eDeviceLocal;;
			break;
		}
	}

	void Renderer::CreateVertexBuffer(Memory& mem, VertexBufferHandle vertHandle, UpdateHint hint)
	{
		BufferInputChunk input;
		input.size = mem.size;
		input.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
		input.memoryProperties = GetMemoryFlags(hint);
		auto& buffer = m_VertexBuffers[vertHandle] = CreateSPtr<Buffer>(input);
		SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(mem.size);

		m_TransferCommandBuffer[0].BeginTransfering();
		m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), buffer.get(), mem.data, mem.size);
		m_TransferCommandBuffer[0].EndTransfering();

	}

	void Renderer::CreateIndexBuffer(Memory& mem, IndexBufferHandle indexHandle)
	{
		BufferInputChunk input;
		input.size = mem.size;
		input.usage = vk::BufferUsageFlagBits::eIndexBuffer |  vk::BufferUsageFlagBits::eTransferDst;
		input.memoryProperties = GetMemoryFlags(UpdateHint::Static);
		auto indexAmount = mem.size / 4;
		auto& buffer = m_IndexBuffers[indexHandle] = CreateSPtr<IndexBuffer>(input, indexAmount, vk::IndexType::eUint32);

		SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(mem.size);

		m_TransferCommandBuffer[0].BeginTransfering();
		m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), buffer.get(), mem.data, mem.size);
		m_TransferCommandBuffer[0].EndTransfering();

	}

	void Renderer::AddFramebuffers(FrameBufferHandle handle, std::vector<vk::Framebuffer>& framebuffers)
	{
		if (m_Framebuffers.find(handle) != m_Framebuffers.end())
		{
			Log::GetLog()->error("Adding existing frame buffer");
		}
		m_Framebuffers[handle] = framebuffers;
	}

	vk::Pipeline Renderer::GetPipeline(const PipelineKey& key, 
		std::array<VertexBinding, RenderItem::MAX_BINDING>& bindings,
		int bindingAmount)
	{
		if (m_Pipelines.find(key) != m_Pipelines.end())
			return m_Pipelines.at(key);

		/*
		struct PipelineKey
		{
			ProgramHandle program;
			RenderState rs;
			PipelineLayoutKey layout;
			RenderPassHandle_ renderPass;
		};
		*/

		PipelineBuilder builder;
		auto& rs = key.rs;
		builder.EnableStencilTest(rs.stencilTest);
		builder.SetDepthTest(rs.depthTest);
		builder.EnableBlend(rs.blend);
		auto& renderPass = m_RenderPasses.at(key.renderPass);

		builder.SetRenderPass(renderPass.m_RenderPass);
		builder.AddExtent(renderPass.m_Extent);

		builder.WriteToDepthBuffer(rs.depthWrite);
		builder.SetSamples(renderPass.samples);
		auto shaderMeta = m_Compiler.m_Programs.at(key.program);
		for (auto& stage : shaderMeta.stages)
		{
			builder.AddShader(stage.module, map(stage.stage));
		}

		builder.SetTopology(map(rs.topology));
		builder.SetPolygoneMode(map(rs.polygon));
		{
			std::vector<vk::DescriptorSetLayout> layouts;
			for (DescriptorLayoutKey& key: shaderMeta.descriptorKey)
			{
				auto& layout = m_DescriptorLayout.at(key);
				layouts.push_back(layout);

			}
			builder.AddDescriptorSetLayouts(layouts);
		}

		for (int i = 0; i < bindingAmount; i++)
		{
			VertexBinding& binding = bindings[i];
			
			auto& vertexLayout = GetVertexLayout(binding.LayoutHandle);
			auto vInputBindDescription = VertexBindingDescription(i, vertexLayout.m_CurrentOffset, vk::VertexInputRate::eVertex);
			
			builder.AddBindingDescription(vInputBindDescription);

			for (int ii =0; ii < vertexLayout.m_Elements.size(); ii++)
			{
				auto& element = vertexLayout.m_Elements[ii];
				auto desc = VertexInputAttributeDescription(i, ii, map(element.type), element.offset);
				builder.AddAttributeDescription(desc);
			}
		}

		

		// for now just 1 - potentially we can merge render passes into one
		builder.SetSubpassIndex(0);



		//void StencilTestOp(vk::CompareOp op, vk::StencilOp fail, vk::StencilOp pass, vk::StencilOp //depthFailOp);
		//void SetControlPoints(int amountPoints);

		builder.AddPipelineLayout(m_PipelineLayout.at(key.layout));
		m_Pipelines[key] = builder.Build();
		return m_Pipelines[key];
	}

	void Renderer::RenderFrame(Frame* render, float deltaTime)
	{

		uint32_t imageIndex;
		auto swapchain = RenderContext::GetSwapchain();
		m_Device->GetDevice().acquireNextImageKHR(swapchain->m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphore[m_CurrentFrame].GetSemaphore(), nullptr, &imageIndex);
		Renderer::Instance()->Wait(m_Fence.GetFence());
		Renderer::Instance()->Reset(m_Fence.GetFence());


		auto& cmd = m_RenderCommandBuffer[imageIndex];
		cmd.BeginRendering();

		for (int i = 0; i < render->CurrentRenderItemIndex; i++)
		{
			RenderItem& renderItem = render->m_renderItem[i];

			View& view = render->Views[renderItem.View];
			assert(renderItem.Program.Valid());
			auto& meta = m_Compiler.m_Programs.at(renderItem.Program);

			if (!view.Fbh.Valid())
			{
				auto& renderPass = m_RenderPasses.at(DEFAULT_RENDER_PASS);

				//struct PipelineKey
				//{
				//	ProgramHandle program;
				//	RenderState rs;
				//	PipelineLayoutKey layout;
				//	RenderPassHandle_ renderPass;
				//};

			}
			// get pipeline
			std::vector<DescriptorLayoutKey>  keys{};
			
			keys.insert(keys.end(),meta.descriptorKey.begin(),meta.descriptorKey.end());

			PipelineKey key ={ renderItem.Program,renderItem.State,keys,DEFAULT_RENDER_PASS};
			
			vk::Pipeline pipeline = GetPipeline(key, renderItem.Bindings, renderItem.currentBinding);
			vk::PipelineLayout layout = m_PipelineLayout.at(key.layout);
			auto& renderPass = m_RenderPasses.at(key.renderPass);
			auto frameBuffer = m_Framebuffers.at(renderPass.m_FrameBufferHandle)[imageIndex];

			cmd.BeginRenderPass(renderPass.m_RenderPass, frameBuffer, renderPass.m_Extent, renderPass.m_ClearValues);
			auto vkCmd = cmd.GetCommandBuffer();
			vkCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

			UpdateUniformBuffer(view.Proj, view.View, m_App->GetExeTime());

			for (int i = 0; i < keys.size(); i++)
			{

				auto& descSet = m_DescriptorSet.at(keys.at(i));
				vkCmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, i, descSet[imageIndex], nullptr);
			}
			vk::Viewport viewport;
			viewport.x = view.Rect[0];
			viewport.y = view.Rect[1];
			viewport.width = view.Rect[2];
			viewport.height = view.Rect[3];
			viewport.minDepth = 0;
			viewport.maxDepth = 1;
			vk::Rect2D scissors;
			scissors.offset = vk::Offset2D{ static_cast<int32_t>(view.Rect[0]),static_cast<int32_t>(view.Rect[1]) };
			scissors.extent = vk::Extent2D{ static_cast<uint32_t>(view.Rect[2]),static_cast<uint32_t>(view.Rect[3]) };
			vkCmd.setViewport(0, 1, &viewport);
			vkCmd.setScissor(0, 1, &scissors);

			std::vector<vk::Buffer> vertexBuffers;
			vertexBuffers.reserve(renderItem.currentBinding);
			std::vector<vk::DeviceSize> offsets(0, renderItem.currentBinding);

			for (auto i = 0; i < renderItem.currentBinding; i++)
			{
				auto buffer = m_VertexBuffers.at(renderItem.Bindings.at(i).VertexHandle)->GetBuffer();
				vertexBuffers.push_back(buffer);
			}
			vkCmd.bindVertexBuffers(0,1,vertexBuffers.data(), offsets.data());
			if (renderItem.IndexBuffer.Valid())
			{
				auto buffer = m_IndexBuffers.at(renderItem.IndexBuffer);
				vkCmd.bindIndexBuffer(buffer->GetBuffer(), 0, buffer->GetIndexType());
				vkCmd.drawIndexed(buffer->GetIndexAmount(), 1, 0, 0, 0);
			}
			else
			{
				// draw without index
			}


			cmd.EndRenderPass();

		}

		cmd.EndRendering();
		
		vk::Semaphore renderFinished;
		renderFinished = m_Graphs[0]->Execute(m_RenderCommandBuffer[m_CurrentFrame], m_CurrentFrame, m_ImageAvailableSemaphore[m_CurrentFrame]);
		
		
		auto& semaphore = m_ImageAvailableSemaphore[m_CurrentFrame].GetSemaphore();
		vk::Semaphore waitSemaphore[] = { renderFinished};
	
			

		

		vk::PresentInfoKHR presentInfo = {};
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = waitSemaphore;

		vk::SwapchainKHR swapChains[] = { swapchain->m_Swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;
		vk::Result present;
		try {
			ZoneScopedN("Presenting");
			present = m_Device->GetPresentQueue().presentKHR(presentInfo);
		}
		catch (vk::OutOfDateKHRError error) {
			present = vk::Result::eErrorOutOfDateKHR;
		}
		if (present == vk::Result::eErrorOutOfDateKHR)
		{
			ZoneScopedN("Recreating swapchain");
			RecreateSwapchain();
		}
		m_CurrentFrame = (m_CurrentFrame + 1) % RenderContext::GetFrameAmount();
		
		FrameMark;
	}

	void Renderer::Wait(const vk::Fence& fence)
	{
		m_Device->GetDevice().waitForFences(fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
	}
	void Renderer::Reset(const vk::Fence& fence)
	{
		m_Device->GetDevice().resetFences(fence);
	}

	
	void Renderer::Flush(std::vector<vk::CommandBuffer> commandBuffers)
	{
		assert(false);
	}
	void Renderer::UpdateUniformBuffer(const glm::mat4& proj, const glm::mat4& view, float time)
	{
		UniformBufferObject ubo{};

		ubo.view = view;
		ubo.proj = proj;
		ubo.time = time;
		memcpy(m_UniformBuffersMapped[m_CurrentFrame], &ubo, sizeof(ubo));
	}

	void Renderer::BeginFrame(Camera& camera, size_t viewportWidth,
		size_t viewportHeight)
	{
		//auto proj = glm::ortho(0.0f, (float)viewportWidth, (float)//viewportHeight,0.f);
		//UpdateUniformBuffer(camera.GetProj(), camera);
	}
	void Renderer::EndFrame()
	{
		//for (auto& e : m_Drawables)
		//{
		//	e.second.clear();
		//}
	}

	static vk::DescriptorSetLayout CreateDescriptorSetLayout(std::vector<vk::DescriptorSetLayoutBinding>& bindings)
	{


		auto device = RenderContext::GetDevice();

		vk::DescriptorSetLayoutCreateInfo layoutInfo;
		layoutInfo.flags = vk::DescriptorSetLayoutCreateFlagBits();
		layoutInfo.bindingCount = bindings.size();
		layoutInfo.pBindings = bindings.data();

		try
		{
			return device->GetDevice().createDescriptorSetLayout(layoutInfo);
		}
		catch (vk::SystemError err)
		{

			Log::GetLog()->error("Failed to create Descriptor Set Layout");
			return nullptr;
		}
	}


	void Renderer::CreatePipelineLayout(PipelineLayoutKey& key)
	{
		std::vector<vk::DescriptorSetLayout> layouts;
		for (auto key : key.layoutKeys)
		{
			layouts.push_back(m_DescriptorLayout.at(key));
		}
		m_PipelineLayout[key] = MakePipelineLayout(RenderContext::GetDevice()->GetDevice(), layouts);
	}

	vk::DescriptorSetLayout Renderer::CreateDescriptorLayout(const DescriptorLayoutKey& key)
	{

		
		auto& bindings = key.bindings;
		std::vector<vk::DescriptorSetLayoutBinding> vkBindings;
		for (auto& v : bindings)
		{
			vkBindings.push_back(DescriptorBindingDescription(v.binding, map(v.kind), mapAccess(v.access), v.count));
		}

		m_DescriptorLayout[key] = CreateDescriptorSetLayout(vkBindings);
		return m_DescriptorLayout[key];
	}
	




	

	Renderer::~Renderer()
	{
		
	}



	void Renderer::CreateInstance()
	{
		InstanceInfo info;
		uint32_t version;
		vkEnumerateInstanceVersion(&version);
		info.vkInfo = vk::ApplicationInfo
		{
			"Voidstar app",
			version,
			"Voidstar",
			version,
			version,
		};
		info.extensionCount = 0;
		const char** extensions = glfwGetRequiredInstanceExtensions(&info.extensionCount);
		for (auto i = 0; i < info.extensionCount; i++) {
			info.extensions.emplace_back(extensions[i]);
		}

		info.extensions.push_back("VK_EXT_debug_utils");
		//tracy
		info.extensions.push_back("VK_EXT_calibrated_timestamps");

		info.layers.push_back("VK_LAYER_KHRONOS_validation");
		RenderContext::CreateInstance(info);
		m_Instance = RenderContext::GetInstance();
		
	}
	
}
