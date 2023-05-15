#include"Prereq.h"
#include "Renderer.h"
#include <set>
#include <fstream>
#include "../Window.h"
#include "../Log.h"
#include "glfw3.h"

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

		CreateSwapchain();
		CreatePipeline();


		CreateFramebuffers();

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
			m_ImageAvailableSemaphore = m_Device.createSemaphore(semaphoreInfo);
			m_RenderFinishedSemaphore = m_Device.createSemaphore(semaphoreInfo);
		}
		catch (vk::SystemError err) {
			
			Log::GetLog()->error("failed to create semaphore");
		}

		vk::FenceCreateInfo fenceInfo = {};
		fenceInfo.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;

		try
		{
			m_InFlightFence = m_Device.createFence(fenceInfo);
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
		renderPassInfo.renderPass = m_RenderPass;
		renderPassInfo.framebuffer = m_SwapchainFrames[imageIndex].framebuffer;
		renderPassInfo.renderArea.offset.x = 0;
		renderPassInfo.renderArea.offset.y = 0;
		renderPassInfo.renderArea.extent = m_SwapchainExtent;

		vk::ClearValue clearColor = { std::array<float, 4>{0.1f, .3f, 0.1f, 1.0f} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		m_CommandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);


		//m_CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, m_SwapchainFrames[imageIndex].descriptorSet, nullptr);

		m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);


	
		//commandBuffer.draw(6, 1, 0, 0);
		m_CommandBuffer.draw(3, 1, 0, 0);

		m_CommandBuffer.endRenderPass();
		
		m_CommandBuffer.end();
	}
	void Renderer::Render()	
	{
		m_Device.waitForFences(m_InFlightFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
		m_Device.resetFences(m_InFlightFence);

		uint32_t imageIndex;

		m_Device.acquireNextImageKHR(m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphore, nullptr, &imageIndex);
		
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

		m_GraphicsQueue.submit(submitInfo, m_InFlightFence);

		vk::PresentInfoKHR presentInfo = {};
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		vk::SwapchainKHR swapChains[] = { m_Swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;
		vk::Result present;
		try {
			present = m_PresentQueue.presentKHR(presentInfo);
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
			 m_CommandBuffer = m_Device.allocateCommandBuffers(allocInfo)[0];
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
		poolInfo.queueFamilyIndex = m_GraphicFamily;
		try 
		{
			m_CommandPool = m_Device.createCommandPool(poolInfo);
			
		}
		catch (vk::SystemError err) 
		{

			Log::GetLog()->error("Failed to create Command Pool");
		}
	}
	void Renderer::CreateFramebuffers()
	{
		for (int i = 0; i < m_SwapchainFrames.size(); ++i) {

			std::vector<vk::ImageView> attachments = {
				m_SwapchainFrames[i].imageView
			};

			vk::FramebufferCreateInfo framebufferInfo;
			framebufferInfo.flags = vk::FramebufferCreateFlags();
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_SwapchainExtent.width;
			framebufferInfo.height = m_SwapchainExtent.height;
			framebufferInfo.layers = 1;

			try
			{
				m_SwapchainFrames[i].framebuffer = m_Device.createFramebuffer(framebufferInfo);

		
				Log::GetLog()->info("Created framebuffer for frame{0} " , i);
				
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
		specs.device = m_Device;
		specs.vertexFilepath = "../Shaders/vertex.spv";
		specs.fragmentFilepath = "../Shaders/fragment.spv";
		specs.swapchainExtent = m_SwapchainExtent;
		specs.swapchainImageFormat = m_SwapchainFormat;

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
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		
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
		for (auto& frame : m_SwapchainFrames) {

			m_Device.destroyImageView(frame.imageView);
			m_Device.destroyFramebuffer(frame.framebuffer);
			

		}
		// cannot not use detroy image on  presentable image
		m_Device.destroySwapchainKHR(m_Swapchain);

	}

	Renderer::~Renderer()
	{
		m_Device.waitIdle();
		DestroySwapchain();

		m_Device.destroyCommandPool(m_CommandPool);
		m_Device.destroySemaphore(m_ImageAvailableSemaphore);
		m_Device.destroySemaphore(m_RenderFinishedSemaphore);
		m_Device.destroyFence(m_InFlightFence);

		m_Device.destroyPipeline(m_Pipeline);
		m_Device.destroyPipelineLayout(m_PipelineLayout);
		m_Device.destroyRenderPass(m_RenderPass);

		m_Instance.destroyDebugUtilsMessengerEXT(m_DebugMessenger, nullptr, m_Dldi);
		m_Instance.destroySurfaceKHR(m_Surface);
		m_Device.destroy();
		m_Instance.destroy();
	}
	void Renderer::CreateDevice()
	{
	

		std::vector<vk::PhysicalDevice> availableDevices = m_Instance.enumeratePhysicalDevices();

		

		/*
		* check if a suitable device can be found
		*/
		const std::vector<const char*> requestedExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		for (vk::PhysicalDevice device : availableDevices)
		{	
			std::set<std::string> requiredExtensions(requestedExtensions.begin(), requestedExtensions.end());
			for (vk::ExtensionProperties& extension : device.enumerateDeviceExtensionProperties()) {

				

				//remove this from the list of required extensions (set checks for equality automatically)
				requiredExtensions.erase(extension.extensionName);
			}

			if (requiredExtensions.empty())
			{
				m_PhysicalDevice = device;
				auto name = m_PhysicalDevice.getProperties().deviceName;
				Log::GetLog()->info("PhysicalDevice is found: {0} ", name);
			

				break;
			}
		}

	


		
		

		std::vector<vk::QueueFamilyProperties> queueFamilies = m_PhysicalDevice.getQueueFamilyProperties();

		uint32_t graphicFamily = -1;
		uint32_t presentFamily = -1;

		int i = 0;
		for (vk::QueueFamilyProperties queueFamily : queueFamilies) {

			/*
			* // Provided by VK_VERSION_1_0
				typedef struct VkQueueFamilyProperties {
				VkQueueFlags    queueFlags;
				uint32_t        queueCount;
				uint32_t        timestampValidBits;
				VkExtent3D      minImageTransferGranularity;
				} VkQueueFamilyProperties;
				queueFlags is a bitmask of VkQueueFlagBits indicating capabilities of the queues in this queue family.
				queueCount is the unsigned integer count of queues in this queue family. Each queue family must support
				at least one queue.
				timestampValidBits is the unsigned integer count of meaningful bits in the timestamps written via
				vkCmdWriteTimestamp. The valid range for the count is 36..64 bits, or a value of 0,
				indicating no support for timestamps. Bits outside the valid range are guaranteed to be zeros.
				minImageTransferGranularity is the minimum granularity supported for image transfer
				operations on the queues in this queue family.
			*/

			/*
			* // Provided by VK_VERSION_1_0
				typedef enum VkQueueFlagBits {
				VK_QUEUE_GRAPHICS_BIT = 0x00000001,
				VK_QUEUE_COMPUTE_BIT = 0x00000002,
				VK_QUEUE_TRANSFER_BIT = 0x00000004,
				VK_QUEUE_SPARSE_BINDING_BIT = 0x00000008,
				} VkQueueFlagBits;
			*/

			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
				graphicFamily = i;

			}


			if (m_PhysicalDevice.getSurfaceSupportKHR(i, m_Surface)) {
				presentFamily =  i;

				
			}


			if (graphicFamily != -1 && presentFamily != -1) {
				break;
			}

			i++;
		}


		std::vector<uint32_t> uniqueIndices;

		uniqueIndices.push_back(graphicFamily);

		if (graphicFamily != presentFamily) {
			uniqueIndices.push_back(presentFamily);
		}

		m_GraphicFamily = graphicFamily;
		m_PresentFamily = presentFamily;


		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfo;
		float queuePriority = 1.0f;
		for (uint32_t queueFamilyIndex : uniqueIndices) {
			queueCreateInfo.push_back(
				vk::DeviceQueueCreateInfo(
					vk::DeviceQueueCreateFlags(), queueFamilyIndex, 1, &queuePriority
				)
			);
		}


		vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();



		std::vector<const char*> enabledLayers;
		
		enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
		
		std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
			vk::DeviceCreateFlags(),
			1, queueCreateInfo.data(),
			enabledLayers.size(), enabledLayers.data(),
			deviceExtensions.size(), deviceExtensions.data(),
			&deviceFeatures
		);

		try {
			m_Device = m_PhysicalDevice.createDevice(deviceInfo);
			
			Log::GetLog()->info( "GPU has been successfully abstracted!");
		
		}
		catch (vk::SystemError err)
		{

			Log::GetLog()->error( "Device creation failed!");
				
		}

		
		
		m_GraphicsQueue  = m_Device.getQueue(graphicFamily, 0);
		m_PresentQueue  = m_Device.getQueue(presentFamily, 0);
		
		
	}
	
	

	void Renderer::CreateSwapchain()
	{
		SwapChainSupportDetails support;

		support.capabilities = m_PhysicalDevice.getSurfaceCapabilitiesKHR(m_Surface);
	
		support.formats = m_PhysicalDevice.getSurfaceFormatsKHR(m_Surface);
		support.presentModes = m_PhysicalDevice.getSurfacePresentModesKHR(m_Surface);



		vk::SurfaceFormatKHR format = GetSurfaceFormat(vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear, support);

	

		vk::PresentModeKHR presentMode = GetPresentMode(vk::PresentModeKHR::eMailbox, support);

		

		vk::Extent2D extent = GetSwapchainExtent(m_ViewportWidth,m_ViewportHeight, support);

		uint32_t imageCount = std::min(
			support.capabilities.maxImageCount,
			support.capabilities.minImageCount + 1
		);

		/*
		* VULKAN_HPP_CONSTEXPR SwapchainCreateInfoKHR(
	  VULKAN_HPP_NAMESPACE::SwapchainCreateFlagsKHR flags_         = {},
	  VULKAN_HPP_NAMESPACE::SurfaceKHR              surface_       = {},
	  uint32_t                                      minImageCount_ = {},
	  VULKAN_HPP_NAMESPACE::Format                  imageFormat_   = VULKAN_HPP_NAMESPACE::Format::eUndefined,
	  VULKAN_HPP_NAMESPACE::ColorSpaceKHR   imageColorSpace_  = VULKAN_HPP_NAMESPACE::ColorSpaceKHR::eSrgbNonlinear,
	  VULKAN_HPP_NAMESPACE::Extent2D        imageExtent_      = {},
	  uint32_t                              imageArrayLayers_ = {},
	  VULKAN_HPP_NAMESPACE::ImageUsageFlags imageUsage_       = {},
	  VULKAN_HPP_NAMESPACE::SharingMode     imageSharingMode_ = VULKAN_HPP_NAMESPACE::SharingMode::eExclusive,
	  uint32_t                              queueFamilyIndexCount_ = {},
	  const uint32_t *                      pQueueFamilyIndices_   = {},
	  VULKAN_HPP_NAMESPACE::SurfaceTransformFlagBitsKHR preTransform_ =
		VULKAN_HPP_NAMESPACE::SurfaceTransformFlagBitsKHR::eIdentity,
	  VULKAN_HPP_NAMESPACE::CompositeAlphaFlagBitsKHR compositeAlpha_ =
		VULKAN_HPP_NAMESPACE::CompositeAlphaFlagBitsKHR::eOpaque,
	  VULKAN_HPP_NAMESPACE::PresentModeKHR presentMode_  = VULKAN_HPP_NAMESPACE::PresentModeKHR::eImmediate,
	  VULKAN_HPP_NAMESPACE::Bool32         clipped_      = {},
	  VULKAN_HPP_NAMESPACE::SwapchainKHR   oldSwapchain_ = {} ) VULKAN_HPP_NOEXCEPT
		*/
		vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
			vk::SwapchainCreateFlagsKHR(), m_Surface, imageCount, format.format, format.colorSpace,
			extent, 1, vk::ImageUsageFlagBits::eColorAttachment
		);


		
		uint32_t queueFamilyIndices[] = { m_GraphicFamily, m_PresentFamily };

		if (m_PresentFamily != m_GraphicFamily) {
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		}

		createInfo.preTransform = support.capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

		
		try {
			m_Swapchain = m_Device.createSwapchainKHR(createInfo);
			Log::GetLog()->info("Swapchain created!");
		}
		catch (vk::SystemError err) {
			Log::GetLog()->error("failed to create swap chain!");
			throw std::runtime_error("failed to create swap chain!");
		}


		// images already exists at this point
		auto images = m_Device.getSwapchainImagesKHR(m_Swapchain);
		m_SwapchainFrames.resize(images.size());
		for (size_t i = 0; i < images.size(); ++i) {

			/*
			* ImageViewCreateInfo( VULKAN_HPP_NAMESPACE::ImageViewCreateFlags flags_ = {},
						   VULKAN_HPP_NAMESPACE::Image                image_ = {},
						   VULKAN_HPP_NAMESPACE::ImageViewType    viewType_  = VULKAN_HPP_NAMESPACE::ImageViewType::e1D,
						   VULKAN_HPP_NAMESPACE::Format           format_    = VULKAN_HPP_NAMESPACE::Format::eUndefined,
						   VULKAN_HPP_NAMESPACE::ComponentMapping components_            = {},
						   VULKAN_HPP_NAMESPACE::ImageSubresourceRange subresourceRange_ = {} ) VULKAN_HPP_NOEXCEPT
				: flags( flags_ )
				, image( image_ )
				, viewType( viewType_ )
				, format( format_ )
				, components( components_ )
				, subresourceRange( subresourceRange_ )
			*/

			vk::ImageViewCreateInfo createInfo = {};
			createInfo.image = images[i];
			createInfo.viewType = vk::ImageViewType::e2D;
			createInfo.format = format.format;
			createInfo.components.r = vk::ComponentSwizzle::eIdentity;
			createInfo.components.g = vk::ComponentSwizzle::eIdentity;
			createInfo.components.b = vk::ComponentSwizzle::eIdentity;
			createInfo.components.a = vk::ComponentSwizzle::eIdentity;
			createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			m_SwapchainFrames[i].image = images[i];
			m_SwapchainFrames[i].imageView = m_Device.createImageView(createInfo);
		}

		m_SwapchainFormat = format.format;
		m_SwapchainExtent = extent;
	}

	vk::SurfaceFormatKHR Renderer::GetSurfaceFormat(vk::Format format, vk::ColorSpaceKHR colorSpace, SwapChainSupportDetails& support)
	{
		for (vk::SurfaceFormatKHR formatSupport : support.formats)
		{
			if (formatSupport.format == format
				&& formatSupport.colorSpace == colorSpace) {
				return formatSupport;
			}
		}

		return support.formats[0];
	}

	vk::PresentModeKHR Renderer::GetPresentMode(vk::PresentModeKHR presentMode, SwapChainSupportDetails& support)
	{
		for (vk::PresentModeKHR presentModeSupport : support.presentModes)
		{
			if (presentMode == vk::PresentModeKHR::eMailbox) {
				presentMode = presentModeSupport;
			}
		}

		return vk::PresentModeKHR::eFifo;
	}
	vk::Extent2D Renderer::GetSwapchainExtent(uint32_t width, uint32_t height, SwapChainSupportDetails& support) {

		auto capabilities = support.capabilities;
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			vk::Extent2D extent = { width, height };

			extent.width = std::min(
				capabilities.maxImageExtent.width,
				std::max(capabilities.minImageExtent.width, extent.width)
			);

			extent.height = std::min(
				capabilities.maxImageExtent.height,
				std::max(capabilities.minImageExtent.height, extent.height)
			);

			return extent;
		}
	}
	void Renderer::CreateSurface()
	{
		VkSurfaceKHR c_style_surface;
		auto rawWindow = m_Window->GetRaw();
		if (glfwCreateWindowSurface(m_Instance,rawWindow, nullptr, &c_style_surface) != VK_SUCCESS)
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
		uint32_t version;
		vkEnumerateInstanceVersion(&version);

		auto appInfo = vk::ApplicationInfo
		{
			"Voidstar app",
			version,
			"Voidstar",
			version,
			version,
		};
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);


		std::vector<const char*> extensions;


		for (auto i = 0; i < glfwExtensionCount; i++) {
			extensions.emplace_back(glfwExtensions[i]);
		}
		
		extensions.push_back("VK_EXT_debug_utils");
		
		

		std::vector<const char*> layers;

		layers.push_back("VK_LAYER_KHRONOS_validation");

		

		vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo(
			vk::InstanceCreateFlags(),
			&appInfo,
			layers.size(), layers.data(), // enabled layers
			extensions.size(), extensions.data() // enabled extensions
		);


		try
		{
			
			m_Instance =  vk::createInstance(createInfo);
			Log::GetLog()->info("Instance is created!");
		}
		catch (vk::SystemError err) 
		{
				Log::GetLog()->error("Failed to create Instance!");
				
		}
	
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
		m_Dldi = vk::DispatchLoaderDynamic(m_Instance, vkGetInstanceProcAddr);

	
		vk::DebugUtilsMessengerCreateInfoEXT createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
			vk::DebugUtilsMessengerCreateFlagsEXT(),
			/*vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |*/ vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
			debugCallback,
			nullptr
		);
		
		try
		{
			m_DebugMessenger = m_Instance.createDebugUtilsMessengerEXT(createInfo, nullptr, m_Dldi);
			Log::GetLog()->info("m_DebugMessenger is created!");
		}
		catch (vk::SystemError& err)
		{
			Log::GetLog()->error("Falied to create m_DebugMessenger!");
		}
		

		
	}
	
}
