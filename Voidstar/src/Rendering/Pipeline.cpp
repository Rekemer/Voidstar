#include"Prereq.h"
#include "Pipeline.h"
#include <fstream>
#include "Device.h"
#include "RenderContext.h"
#include "../Log.h"
namespace Voidstar
{
	std::vector<char> ReadFile(std::string_view filename);
	vk::ShaderModule CreateModule(std::string_view filename, vk::Device device);
	static vk::PipelineLayout MakePipelineLayout(vk::Device device, std::vector<vk::DescriptorSetLayout> layout);

	UPtr<Pipeline> Pipeline::CreateGraphicsPipeline(GraphicsPipelineSpecification& spec, vk::PrimitiveTopology topology, vk::Format depthFormat, vk::RenderPass renderpass ,int subpass,bool writeToDepthBuffer, vk::PolygonMode polygonMode)
	{
		auto pipeline = CreateUPtr<Pipeline>();


		/*
		* Build and return a graphics pipeline based on the given info.
		*/

		//The info for the graphics pipeline
		vk::GraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.flags = vk::PipelineCreateFlags();


		//Shader stages, to be populated later
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

		//Vertex Input


		//vk::VertexInputBindingDescription bindingDescription = vkMesh::getPosColorBindingDescription();
		//auto  attributeDescriptions = vkMesh::getPosColorAttributeDescriptions();
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};

		vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
		vertexInputInfo.vertexBindingDescriptionCount = spec.bindingDescription.size();
		vertexInputInfo.pVertexBindingDescriptions = spec.bindingDescription.data();

		vertexInputInfo.vertexAttributeDescriptionCount = spec.attributeDescription.size();
		vertexInputInfo.pVertexAttributeDescriptions = spec.attributeDescription.data();

		pipelineInfo.pVertexInputState = &vertexInputInfo;

		//Input Assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
		inputAssemblyInfo.topology = topology;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

		vk::PipelineTessellationStateCreateInfo tesselationState;
		if (spec.tessCFilepath != "")
		{
			tesselationState.patchControlPoints = 4;

			pipelineInfo.pTessellationState = &tesselationState;
		}









		//Viewport and Scissor
		vk::Viewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)spec.swapchainExtent.width;
		viewport.height = (float)spec.swapchainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vk::Rect2D scissor = {};
		scissor.offset.x = 0.0f;
		scissor.offset.y = 0.0f;
		scissor.extent = spec.swapchainExtent;
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
		rasterizer.polygonMode = polygonMode;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = vk::CullModeFlagBits::eNone;
		rasterizer.frontFace = vk::FrontFace::eClockwise;
		rasterizer.depthBiasEnable = VK_FALSE; //Depth bias can be useful in shadow maps.
		pipelineInfo.pRasterizationState = &rasterizer;



		vk::ShaderModule vertexShader = CreateModule(
			spec.vertexFilepath, spec.device
		);
		{
			vk::PipelineShaderStageCreateInfo vertexShaderInfo = {};
			vertexShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
			vertexShaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
			vertexShaderInfo.module = vertexShader;
			vertexShaderInfo.pName = "main";
			shaderStages.push_back(vertexShaderInfo);
		}



		vk::ShaderModule fragmentShader = CreateModule(
			spec.fragmentFilepath, spec.device
		);
		{
			vk::PipelineShaderStageCreateInfo fragmentShaderInfo = {};
			fragmentShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
			fragmentShaderInfo.stage = vk::ShaderStageFlagBits::eFragment;
			fragmentShaderInfo.module = fragmentShader;
			fragmentShaderInfo.pName = "main";
			shaderStages.push_back(fragmentShaderInfo);
		}

		vk::ShaderModule tessControl, tessEvaulation;
		if (spec.tessCFilepath != "")
		{

			tessControl = CreateModule(
				spec.tessCFilepath, spec.device
			);
			{
				vk::PipelineShaderStageCreateInfo tessShaderInfo = {};
				tessShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
				tessShaderInfo.stage = vk::ShaderStageFlagBits::eTessellationControl;
				tessShaderInfo.module = tessControl;
				tessShaderInfo.pName = "main";
				shaderStages.push_back(tessShaderInfo);
			}
			tessEvaulation = CreateModule(
				spec.tessEFilepath, spec.device
			);
			{
				vk::PipelineShaderStageCreateInfo tessShaderInfo = {};
				tessShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
				tessShaderInfo.stage = vk::ShaderStageFlagBits::eTessellationEvaluation;
				tessShaderInfo.module = tessEvaulation;
				tessShaderInfo.pName = "main";
				shaderStages.push_back(tessShaderInfo);
			}
		}
		vk::ShaderModule computeModule;
		if (spec.computeFilepath != "")
		{

			computeModule = CreateModule(
				spec.computeFilepath, spec.device
			);
			{
				vk::PipelineShaderStageCreateInfo computeShaderInfo = {};
				computeShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
				computeShaderInfo.stage = vk::ShaderStageFlagBits::eCompute;
				computeShaderInfo.module = computeModule;
				computeShaderInfo.pName = "main";
				shaderStages.push_back(computeShaderInfo);
			}


		}





		pipelineInfo.stageCount = shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();




		vk::PipelineDepthStencilStateCreateInfo depthState;
		depthState.flags = vk::PipelineDepthStencilStateCreateFlags();
		depthState.depthTestEnable = true;
		depthState.depthWriteEnable = writeToDepthBuffer;
		depthState.depthCompareOp = vk::CompareOp::eLess;
		depthState.depthBoundsTestEnable = false;
		depthState.stencilTestEnable = false;
		pipelineInfo.pDepthStencilState = &depthState;

		//Multisampling
		vk::PipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.flags = vk::PipelineMultisampleStateCreateFlags();
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = spec.samples;
		pipelineInfo.pMultisampleState = &multisampling;

		//Color Blend
		vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;


		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
		colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
		colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;



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


		vk::PipelineLayout m_pipelineLayout = MakePipelineLayout(spec.device, spec.descriptorSetLayout);
		pipelineInfo.layout = m_pipelineLayout;




		//Renderpass

		pipelineInfo.renderPass = renderpass;
		pipelineInfo.subpass = subpass;


		//Extra stuff
		pipelineInfo.basePipelineHandle = nullptr;

		//Make the Pipeline

		vk::Pipeline graphicsPipeline;
		try
		{
			graphicsPipeline = (spec.device.createGraphicsPipeline(nullptr, pipelineInfo)).value;
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to create Pipeline");
		}

		pipeline->m_PipelineLayout= m_pipelineLayout;
		pipeline->m_Pipeline= graphicsPipeline;

		//Finally clean up by destroying shader modules
		spec.device.destroyShaderModule(vertexShader);
		spec.device.destroyShaderModule(fragmentShader);
		if (tessControl)
		{
			spec.device.destroyShaderModule(tessControl);
			spec.device.destroyShaderModule(tessEvaulation);
		}

		if (computeModule)
		{
			spec.device.destroyShaderModule(computeModule);

		}
		Log::GetLog()->info("Pipeline is Created!");
		return pipeline;
	}
	UPtr<Pipeline> Pipeline::CreateComputePipeline(std::string computeShader, std::vector<vk::DescriptorSetLayout>& layouts)
	{
		UPtr<Pipeline> pipeline = CreateUPtr<Pipeline>();

		auto device = RenderContext::GetDevice();
		auto computeShaderModule = CreateModule(computeShader, device->GetDevice());

		vk::PipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.flags = vk::PipelineShaderStageCreateFlags();
		computeShaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
		computeShaderStageInfo.module = computeShaderModule;
		computeShaderStageInfo.pName = "main";



		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipeline->m_PipelineLayout = device->GetDevice().createPipelineLayout(pipelineLayoutInfo, nullptr);


		vk::ComputePipelineCreateInfo pipelineInfo{};

		pipelineInfo.sType = vk::StructureType::eComputePipelineCreateInfo;
		pipelineInfo.layout = pipeline->m_PipelineLayout;
		pipelineInfo.stage = computeShaderStageInfo;
		pipeline->m_Pipeline = device->GetDevice().createComputePipeline(nullptr, pipelineInfo).value;



		vkDestroyShaderModule(device->GetDevice(), computeShaderModule, nullptr);



		return pipeline;
	}
	Pipeline::~Pipeline()
	{
		auto& device = RenderContext::GetDevice()->GetDevice();
		device.waitIdle();
		device.destroyPipeline(m_Pipeline);
		device.destroyPipelineLayout(m_PipelineLayout);
	
	}

	vk::ShaderModule CreateModule(std::string_view filename, vk::Device device) {

		std::vector<char> sourceCode = ReadFile(filename);
		vk::ShaderModuleCreateInfo moduleInfo = {};
		moduleInfo.flags = vk::ShaderModuleCreateFlags();
		moduleInfo.codeSize = sourceCode.size();
		moduleInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCode.data());

		try {
			return device.createShaderModule(moduleInfo);
		}
		catch (vk::SystemError err) {

			Log::GetLog()->error("Failed to create shader module for{0}", filename);

		}
	}
	

	static vk::PipelineLayout MakePipelineLayout(vk::Device device, std::vector<vk::DescriptorSetLayout> layout) {

		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo.flags = vk::PipelineLayoutCreateFlags();
		layoutInfo.setLayoutCount = layout.size();
		layoutInfo.pSetLayouts = layout.data();

		layoutInfo.pushConstantRangeCount = 0;
		try
		{
			return device.createPipelineLayout(layoutInfo);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to create pipeline layout!");
		}
	}

	std::vector<char> ReadFile(std::string_view filename)
	{

		std::ifstream file(filename.data(), std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			Log::GetLog()->error("Failed to load {0}", filename);
		}

		size_t filesize{ static_cast<size_t>(file.tellg()) };

		std::vector<char> buffer(filesize);
		file.seekg(0);
		file.read(buffer.data(), filesize);

		file.close();
		return buffer;
	}

	void PipelineBuilder::SetDevice(vk::Device device)
	{
		m_Device = device;
	}


	void PipelineBuilder::AddShader(std::string_view path, vk::ShaderStageFlagBits type)
	{
		vk::ShaderModule vertexShader = CreateModule(
			path,m_Device
		);
		m_Modules.push_back(vertexShader);
		{
			vk::PipelineShaderStageCreateInfo vertexShaderInfo = {};
			vertexShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
			//vertexShaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
			vertexShaderInfo.stage = type;
			vertexShaderInfo.module = vertexShader;
			vertexShaderInfo.pName = "main";
			m_ShaderStages.push_back(vertexShaderInfo);
		}
	}


	void PipelineBuilder::AddBindingDescription(std::vector<vk::VertexInputBindingDescription>& bindings)
	{
		m_Bindings = bindings;
	}

	void PipelineBuilder::AddAttributeDescription(std::vector<vk::VertexInputAttributeDescription>& attributes)
	{
		m_Attributes = attributes;
	}

	void PipelineBuilder::AddDescriptorLayouts(std::vector<vk::DescriptorSetLayout>& layouts)
	{
		m_DescriptorSetLayouts = layouts;
	}

	void PipelineBuilder::AddExtent(vk::Extent2D size)
	{
		m_Extent =  size;
	}

	void PipelineBuilder::AddImageFormat(vk::Format swapchainImageFormat)
	{
		m_SwapchainImageFormat = swapchainImageFormat;
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

	void PipelineBuilder::StencilTestOp(vk::CompareOp op)
	{
		m_StencilOp = op;
	}

	void PipelineBuilder::SetSamples(vk::SampleCountFlagBits samples)
	{
		m_Samples = samples;
	}

	void PipelineBuilder::SetRenderPass(vk::RenderPass renderPass)
	{
		m_RenderPass = renderPass;
	}

	void PipelineBuilder::SetSubpassAmount(int numberOfSubpass)
	{
		m_SubpassNumber = numberOfSubpass;
	}

	UPtr<Pipeline>  PipelineBuilder::Build()
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
		if (m_PatchControlPoints != -1 )
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
		scissor.extent =m_Extent;
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
		depthState.depthTestEnable = true;
		depthState.depthWriteEnable = m_WriteToDepthBuffer;
		depthState.depthCompareOp = vk::CompareOp::eLess;
		depthState.depthBoundsTestEnable = false;
		depthState.stencilTestEnable = m_StencilTest;
		depthState.back.compareOp = m_StencilOp;
		depthState.back.compareMask = 0xff;
		depthState.back.writeMask= 0xff;
		depthState.back.reference = m_StencilRefNumber;
		depthState.front = depthState.back;

		pipelineInfo.pDepthStencilState = &depthState;

		//Multisampling
		vk::PipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.flags = vk::PipelineMultisampleStateCreateFlags();
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = m_Samples;
		pipelineInfo.pMultisampleState = &multisampling;

		//Color Blend
		vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;


		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
		colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
		colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;



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


		vk::PipelineLayout m_pipelineLayout = MakePipelineLayout(m_Device, m_DescriptorSetLayouts);
		pipelineInfo.layout = m_pipelineLayout;




		//Renderpass

		pipelineInfo.renderPass = m_RenderPass;
		pipelineInfo.subpass = m_SubpassNumber;


		//Extra stuff
		pipelineInfo.basePipelineHandle = nullptr;

		//Make the Pipeline

		vk::Pipeline graphicsPipeline;
		try
		{
			graphicsPipeline = m_Device.createGraphicsPipeline(nullptr, pipelineInfo).value;
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to create Pipeline");
		}

		pipeline->m_PipelineLayout = m_pipelineLayout;
		pipeline->m_Pipeline = graphicsPipeline;


		for (auto mod : m_Modules)
		{
			m_Device.destroyShaderModule(mod);
		}

		Log::GetLog()->info("Pipeline is Created!");
		return pipeline;
	}


}