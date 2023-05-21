#include"Prereq.h"
#include "Renderer.h"
#include <set>
#include <fstream>
#include "../Window.h"
#include "../Log.h"
#include "glfw3.h"
#include "Instance.h"
#include "Device.h"
#include "Swapchain.h"
#include "SupportStruct.h"
#include "Vertex.h"
#include "VertexBuffer.h"

namespace Voidstar
{
	Renderer::Renderer(size_t screenWidth, size_t screenHeight, Window* window) :
		m_Window{window}, m_ViewportWidth(screenWidth), m_ViewportHeight(screenHeight)
	{
		
		// create instance
		CreateInstance();

		CreateDebugMessenger();

		CreateSurface();

		CreateDevice();

		SwapChainSupportDetails support;
		support.devcie = m_Device;
		support.surface = &m_Surface;
		support.capabilities = m_Device->GetDevicePhys().getSurfaceCapabilitiesKHR(m_Surface);
		support.formats = m_Device->GetDevicePhys().getSurfaceFormatsKHR(m_Surface);
		support.presentModes = m_Device->GetDevicePhys().getSurfacePresentModesKHR(m_Surface);
		support.viewportWidth = m_ViewportWidth;
		support.viewportHeight = m_ViewportHeight;

		m_Swapchain = Swapchain::Create(support);


		CreatePipeline();


		CreateFramebuffers();


		const float cubeVertices[] = {
			// Front face
			-0.5f, -0.5f,  0.5f, 1.f,1.f,1.f,1.f, // Vertex 0
			 0.5f, -0.5f,  0.5f, 1.f,1.f,1.f,1.f, // Vertex 1
			 0.5f,  0.5f,  0.5f, 1.f,1.f,1.f,1.f, // Vertex 2
			-0.5f,  0.5f,  0.5f, 1.f,1.f,1.f,1.f, // Vertex 3
								 
			// Back face		 
			-0.5f, -0.5f, -0.5f, 1.f,1.f,1.f,1.f, // Vertex 4
			 0.5f, -0.5f, -0.5f, 1.f,1.f,1.f,1.f, // Vertex 5
			 0.5f,  0.5f, -0.5f, 1.f,1.f,1.f,1.f, // Vertex 6
			-0.5f,  0.5f, -0.5f, 1.f,1.f,1.f,1.f , // Vertex 7
								 
			// Left face		 
			-0.5f, -0.5f,  0.5f, 1.f,1.f,1.f,1.f,// Vertex 8
			-0.5f,  0.5f,  0.5f, 1.f,1.f,1.f,1.f,// Vertex 9
			-0.5f,  0.5f, -0.5f, 1.f,1.f,1.f,1.f,// Vertex 10
			-0.5f, -0.5f, -0.5f, 1.f,1.f,1.f,1.f, // Vertex 11
								 
			// Right face		 
			 0.5f, -0.5f,  0.5f, 1.f,1.f,1.f,1.f, // Vertex 12
			 0.5f,  0.5f,  0.5f, 1.f,1.f,1.f,1.f, // Vertex 13
			 0.5f,  0.5f, -0.5f, 1.f,1.f,1.f,1.f, // Vertex 14
			 0.5f, -0.5f, -0.5f, 1.f,1.f,1.f,1.f,  // Vertex 15

			 // Top face
			 - 0.5f,  0.5f,  0.5f,1.f,1.f,1.f,1.f, // Vertex 16
			  0.5f,  0.5f,  0.5f,1.f,1.f,1.f,1.f, // Vertex 17
			  0.5f,  0.5f, -0.5f,1.f,1.f,1.f,1.f, // Vertex 18
			 -0.5f,  0.5f, -0.5f,1.f,1.f,1.f,1.f , // Vertex 19

			// Bottom face
			-0.5f, -0.5f,  0.5f, 1.f,1.f,1.f,1.f, // Vertex 20
			 0.5f, -0.5f,  0.5f, 1.f,1.f,1.f,1.f,// Vertex 21
			 0.5f, -0.5f, -0.5f, 1.f,1.f,1.f,1.f,// Vertex 22
			-0.5f, -0.5f, -0.5f, 1.f,1.f,1.f,1.f // Vertex 23
		};

		BufferInputChunk input;


		input.size = sizeof(cubeVertices);
		input.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		input.usage = vk::BufferUsageFlagBits::eVertexBuffer;
		input.device = m_Device;
		
		


		m_Buffer = new VertexBuffer(input);
		void* vertexData = static_cast<void*>(const_cast<float*>(cubeVertices));
		m_Buffer->SetData(vertexData);
		CreateCommandPool();
		CreateCommandBuffer();
		CreateSyncObjects();
	}

	void Renderer::CreateSyncObjects()
	{	
		vk::SemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.flags = vk::SemaphoreCreateFlags();

		try
		{
			m_ImageAvailableSemaphore = m_Device->GetDevice().createSemaphore(semaphoreInfo);
			m_RenderFinishedSemaphore = m_Device->GetDevice().createSemaphore(semaphoreInfo);
		}
		catch (vk::SystemError err) {
			
			Log::GetLog()->error("failed to create semaphore");
		}

		vk::FenceCreateInfo fenceInfo = {};
		fenceInfo.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;

		try
		{
			m_InFlightFence = m_Device->GetDevice().createFence(fenceInfo);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("failed to create fence");
		}


	
	}
	void Renderer::RecordCommandBuffer(uint32_t imageIndex)
	{
		vk::CommandBufferBeginInfo beginInfo = {};

	
		m_CommandBuffer.begin(beginInfo);
	

		vk::RenderPassBeginInfo renderPassInfo = {};
		auto swapChainExtent = m_Swapchain->GetExtent();
		renderPassInfo.renderPass = m_RenderPass;
		renderPassInfo.framebuffer = m_Swapchain->GetFrames()[imageIndex].framebuffer;
		renderPassInfo.renderArea.offset.x = 0;
		renderPassInfo.renderArea.offset.y = 0;
		renderPassInfo.renderArea.extent = swapChainExtent;

		vk::ClearValue clearColor = { std::array<float, 4>{0.1f, .3f, 0.1f, 1.0f} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		m_CommandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);


		//m_CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, m_SwapchainFrames[imageIndex].descriptorSet, nullptr);

		m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
		vk::DeviceSize offsets[] = { 0 };
		vk::Buffer vertexBuffers[] = { m_Buffer->GetBuffer()};
		m_CommandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
	
		//commandBuffer.draw(6, 1, 0, 0);
		m_CommandBuffer.draw(23, 1, 0, 0);

		m_CommandBuffer.endRenderPass();
		
		m_CommandBuffer.end();
	}
	void Renderer::Render()	
	{
		m_Device->GetDevice().waitForFences(m_InFlightFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
		m_Device->GetDevice().resetFences(m_InFlightFence);

		uint32_t imageIndex;
		auto swapchain = m_Swapchain->GetSwapChain();
		m_Device->GetDevice().acquireNextImageKHR(swapchain, UINT64_MAX, m_ImageAvailableSemaphore, nullptr, &imageIndex);
		
		m_CommandBuffer.reset();

		RecordCommandBuffer(imageIndex);

		vk::SubmitInfo submitInfo = {};

		vk::Semaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;

		vk::Semaphore signalSemaphores[] = { m_RenderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		m_Device->GetGraphicsQueue().submit(submitInfo, m_InFlightFence);

		vk::PresentInfoKHR presentInfo = {};
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		vk::SwapchainKHR swapChains[] = { swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;
		vk::Result present;
		try {
			present = m_Device->GetPresentQueue().presentKHR(presentInfo);
		}
		catch (vk::OutOfDateKHRError error) {
			present = vk::Result::eErrorOutOfDateKHR;
		}

	}

	void Renderer::CreateCommandBuffer()
	{
		vk::CommandBufferAllocateInfo allocInfo = {};
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = 1;



		//Make a "main" command buffer for the engine
		try 
		{
			 m_CommandBuffer = m_Device->GetDevice().allocateCommandBuffers(allocInfo)[0];
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to allocate  command buffer ");
		}
	}

	void Renderer::CreateCommandPool()
	{
		

		vk::CommandPoolCreateInfo poolInfo;
		poolInfo.flags = vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		poolInfo.queueFamilyIndex = m_Device->GetGraphicsIndex();
		try 
		{
			m_CommandPool = m_Device->GetDevice().createCommandPool(poolInfo);
			
		}
		catch (vk::SystemError err) 
		{

			Log::GetLog()->error("Failed to create Command Pool");
		}
	}
	void Renderer::CreateFramebuffers()
	{
		auto& frames = m_Swapchain->GetFrames();
		auto swapChainExtent = m_Swapchain->GetExtent();
		for (int i = 0; i < frames.size(); ++i) {

			std::vector<vk::ImageView> attachments = {
				frames[i].imageView
			};

			vk::FramebufferCreateInfo framebufferInfo;
			framebufferInfo.flags = vk::FramebufferCreateFlags();
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			try
			{
				frames[i].framebuffer = m_Device->GetDevice().createFramebuffer(framebufferInfo);
				
			}
			catch (vk::SystemError err)
			{
				Log::GetLog()->error("Failed to create framebuffer for frame{0} ",i);	
			}

		}
	}

	void Renderer::CreatePipeline()
	{
		GraphicsPipelineSpecification specs;
		auto swapchainFormat = m_Swapchain->GetFormat();
		auto swapChainExtent = m_Swapchain->GetExtent();
		specs.device = m_Device->GetDevice();
		specs.vertexFilepath = "../Shaders/vertex.spv";
		specs.fragmentFilepath = "../Shaders/fragment.spv";
		specs.swapchainExtent = swapChainExtent;
		specs.swapchainImageFormat = swapchainFormat;
		specs.bindingDescription = Vertex::GetBindingDescription();
		specs.attributeDescription = Vertex::GetAttributeDescriptions();

		auto pipline = CreatePipeline(specs);
		m_Pipeline = pipline.pipeline;
		m_PipelineLayout= pipline.layout;
		m_RenderPass= pipline.renderpass;
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

	vk::RenderPass MakeRenderPass(vk::Device device, vk::Format swapchainImageFormat) {

		//Define a general attachment, with its load/store operations
		vk::AttachmentDescription colorAttachment = {};
		colorAttachment.flags = vk::AttachmentDescriptionFlags();
		colorAttachment.format = swapchainImageFormat;
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

		//Declare that attachment to be color buffer 0 of the framebuffer
		vk::AttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		//Renderpasses are broken down into subpasses, there's always at least one.
		vk::SubpassDescription subpass = {};
		subpass.flags = vk::SubpassDescriptionFlags();
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;


		vk::SubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		// define order of subpasses?
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.srcAccessMask = vk::AccessFlagBits::eNone;
		// define rights of subpasses?
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

		//Now create the renderpass
		vk::RenderPassCreateInfo renderpassInfo = {};
		// to be able to map NDC to screen coordinates - Viewport ans Scissors Transform
		renderpassInfo.flags = vk::RenderPassCreateFlags();
		renderpassInfo.attachmentCount = 1;
		renderpassInfo.pAttachments = &colorAttachment;
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


	vk::PipelineLayout MakePipelineLayout(vk::Device device, std::vector<vk::DescriptorSetLayout> layout) {

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

	GraphicsPipeline Renderer::CreatePipeline(GraphicsPipelineSpecification& spec)
	{
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
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &spec.bindingDescription;
		
		vertexInputInfo.vertexAttributeDescriptionCount = spec.attributeDescription.size();
		vertexInputInfo.pVertexAttributeDescriptions = spec.attributeDescription.data();

		pipelineInfo.pVertexInputState = &vertexInputInfo;

		//Input Assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
		inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

		
		
		vk::ShaderModule vertexShader = CreateModule(
			spec.vertexFilepath, spec.device
		);
		vk::PipelineShaderStageCreateInfo vertexShaderInfo = {};
		vertexShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
		vertexShaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
		vertexShaderInfo.module = vertexShader;
		vertexShaderInfo.pName = "main";
		shaderStages.push_back(vertexShaderInfo);

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

		//Rasterizer
		vk::PipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.flags = vk::PipelineRasterizationStateCreateFlags();
		rasterizer.depthClampEnable = VK_FALSE; //discard out of bounds fragments, don't clamp them
		rasterizer.rasterizerDiscardEnable = VK_FALSE; //This flag would disable fragment output
		rasterizer.polygonMode = vk::PolygonMode::eFill;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = vk::CullModeFlagBits::eNone;
		rasterizer.frontFace = vk::FrontFace::eClockwise;
		rasterizer.depthBiasEnable = VK_FALSE; //Depth bias can be useful in shadow maps.
		pipelineInfo.pRasterizationState = &rasterizer;

		
		vk::ShaderModule fragmentShader = CreateModule(
			spec.fragmentFilepath, spec.device
		);
		vk::PipelineShaderStageCreateInfo fragmentShaderInfo = {};
		fragmentShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
		fragmentShaderInfo.stage = vk::ShaderStageFlagBits::eFragment;
		fragmentShaderInfo.module = fragmentShader;
		fragmentShaderInfo.pName = "main";
		shaderStages.push_back(fragmentShaderInfo);
		//Now both shaders have been made, we can declare them to the pipeline info
		pipelineInfo.stageCount = shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();

		//Multisampling
		vk::PipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.flags = vk::PipelineMultisampleStateCreateFlags();
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
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
		
		vk::RenderPass renderpass = MakeRenderPass(spec.device, spec.swapchainImageFormat);
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

		GraphicsPipeline output;
		output.layout = m_pipelineLayout;
		output.renderpass = renderpass;
		output.pipeline = graphicsPipeline;

		//Finally clean up by destroying shader modules
		spec.device.destroyShaderModule(vertexShader);
		spec.device.destroyShaderModule(fragmentShader);
		Log::GetLog()->info("Pipeline is Created!");
		return output;


	}
	void Renderer::DestroySwapchain()
	{
		

	}

	Renderer::~Renderer()
	{
		m_Device->GetDevice().waitIdle();
		delete m_Swapchain;
		delete m_Buffer;
		

		m_Device->GetDevice().destroyCommandPool(m_CommandPool);
		m_Device->GetDevice().destroySemaphore(m_ImageAvailableSemaphore);
		m_Device->GetDevice().destroySemaphore(m_RenderFinishedSemaphore);
		m_Device->GetDevice().destroyFence(m_InFlightFence);

		m_Device->GetDevice().destroyPipeline(m_Pipeline);
		m_Device->GetDevice().destroyPipelineLayout(m_PipelineLayout);
		m_Device->GetDevice().destroyRenderPass(m_RenderPass);

		m_Instance->GetInstance().destroyDebugUtilsMessengerEXT(m_DebugMessenger, nullptr, m_Dldi);
		m_Instance->GetInstance().destroySurfaceKHR(m_Surface);
		m_Device->GetDevice().destroy();
		m_Instance->GetInstance().destroy();
	}
	void Renderer::CreateDevice()
	{
		m_Device = Device::Create(m_Instance, m_Surface);
	}
	
	

	void Renderer::CreateSwapchain()
	{

		
	}

	
	void Renderer::CreateSurface()
	{
		VkSurfaceKHR c_style_surface;
		auto rawWindow = m_Window->GetRaw();
		if (glfwCreateWindowSurface(m_Instance->GetInstance(), rawWindow, nullptr, &c_style_surface) != VK_SUCCESS)
		{
		
			std::cout << "Failed to abstract glfw surface for Vulkan\n";
			Log::GetLog()->error("Failed to abstract glfw surface for Vulkan");
		}
		else 
		{
			Log::GetLog()->info("Successfully abstracted glfw surface for Vulkan");
			
		}
		//copy constructor converts to hpp convention
		m_Surface = c_style_surface;
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

		info.layers.push_back("VK_LAYER_KHRONOS_validation");

		m_Instance = Instance::Create(info);

	
	}

	inline VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	void Renderer::CreateDebugMessenger()
	{
		m_Dldi = vk::DispatchLoaderDynamic(m_Instance->GetInstance(), vkGetInstanceProcAddr);

	
		vk::DebugUtilsMessengerCreateInfoEXT createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
			vk::DebugUtilsMessengerCreateFlagsEXT(),
			/*vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |*/ vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
			debugCallback,
			nullptr
		);
		
		try
		{
			m_DebugMessenger = m_Instance->GetInstance().createDebugUtilsMessengerEXT(createInfo, nullptr, m_Dldi);
			Log::GetLog()->info("m_DebugMessenger is created!");
		}
		catch (vk::SystemError& err)
		{
			Log::GetLog()->error("Falied to create m_DebugMessenger!");
		}
		

		
	}
	
}
