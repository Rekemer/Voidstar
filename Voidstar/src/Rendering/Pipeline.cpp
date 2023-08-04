#include"Prereq.h"
#include "Pipeline.h"
#include <fstream>
#include "Device.h"
#include "RenderContext.h"
#include "../Log.h"
namespace Voidstar
{
	std::vector<char> ReadFile(std::string filename);
	vk::ShaderModule CreateModule(std::string filename, vk::Device device);
	static vk::PipelineLayout MakePipelineLayout(vk::Device device, std::vector<vk::DescriptorSetLayout> layout);
	static vk::RenderPass MakeRenderPass(vk::Device device, vk::Format swapchainImageFormat, vk::Format depthFormat);

	UPtr<Pipeline> Pipeline::CreateGraphicsPipeline(GraphicsPipelineSpecification& spec, vk::PrimitiveTopology topology, vk::Format depthFormat,vk::PolygonMode polygonMode)
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
		depthState.depthWriteEnable = true;
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

		vk::RenderPass renderpass = MakeRenderPass(spec.device, spec.swapchainImageFormat, depthFormat);
		pipelineInfo.renderPass = renderpass;
		pipelineInfo.subpass = 0;


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
		pipeline->m_RenderPass= renderpass;
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
	Pipeline::~Pipeline()
	{
		auto& device = RenderContext::GetDevice()->GetDevice();
		device.destroyPipeline(m_Pipeline);
		device.destroyPipelineLayout(m_PipelineLayout);
		device.destroyRenderPass(m_RenderPass);
	}

	vk::ShaderModule CreateModule(std::string filename, vk::Device device) {

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
	static vk::RenderPass MakeRenderPass(vk::Device device, vk::Format swapchainImageFormat, vk::Format depthFormat) {

		auto samples = RenderContext::GetDevice()->GetSamples();
		//Define a general attachment, with its load/store operations
		vk::AttachmentDescription msaaAttachment = {};
		msaaAttachment.flags = vk::AttachmentDescriptionFlags();
		msaaAttachment.format = swapchainImageFormat;
		msaaAttachment.samples = samples;
		msaaAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		msaaAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		msaaAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		msaaAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		msaaAttachment.initialLayout = vk::ImageLayout::eUndefined;
		msaaAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
		//msaaAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;


		vk::AttachmentReference msaaAttachmentRef = {};
		msaaAttachmentRef.attachment = 0;
		msaaAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
		//msaaAttachmentRef.layout = vk::ImageLayout::ePresentSrcKHR;



		vk::AttachmentDescription depthAttachment = {};
		depthAttachment.flags = vk::AttachmentDescriptionFlags();
		depthAttachment.format = depthFormat;
		depthAttachment.samples = samples;
		depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
		depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::AttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		//Define a general attachment, with its load/store operations
		vk::AttachmentDescription colorAttachmentResolve = {};
		colorAttachmentResolve.flags = vk::AttachmentDescriptionFlags();
		colorAttachmentResolve.format = swapchainImageFormat;
		colorAttachmentResolve.samples = vk::SampleCountFlagBits::e1;
		colorAttachmentResolve.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachmentResolve.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachmentResolve.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachmentResolve.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachmentResolve.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachmentResolve.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

		//Declare that attachment to be color buffer 0 of the framebuffer
		vk::AttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 2;
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;


		//Renderpasses are broken down into subpasses, there's always at least one.
		vk::SubpassDescription subpass = {};
		subpass.flags = vk::SubpassDescriptionFlags();
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &msaaAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentRef;


		vk::SubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		// define order of subpasses?
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.srcAccessMask = vk::AccessFlagBits::eNone;
		// define rights of subpasses?
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		//Now create the renderpass
		vk::RenderPassCreateInfo renderpassInfo = {};

		std::vector<vk::AttachmentDescription> attachments = { msaaAttachment,depthAttachment,colorAttachmentResolve };
		// to be able to map NDC to screen coordinates - Viewport ans Scissors Transform
		renderpassInfo.flags = vk::RenderPassCreateFlags();
		renderpassInfo.attachmentCount = attachments.size();
		renderpassInfo.pAttachments = attachments.data();
		renderpassInfo.subpassCount = 1;
		renderpassInfo.pSubpasses = &subpass;
		renderpassInfo.dependencyCount = 1;
		renderpassInfo.pDependencies = &dependency;

		try {
			return device.createRenderPass(renderpassInfo);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to create renderpass!");
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

	std::vector<char> ReadFile(std::string filename)
	{

		std::ifstream file(filename, std::ios::ate | std::ios::binary);

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

}