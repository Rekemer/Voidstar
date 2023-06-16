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
#include "Buffer.h"
#include "IndexBuffer.h"
#include "RenderContext.h"
#include "DescriptorSetLayout.h"
#include "Camera.h"
#include "Image.h"
#include "DescriptorPool.h"
#include "Model.h"
#include "../Application.h"

namespace Voidstar
{
	Renderer::Renderer(size_t screenWidth, size_t screenHeight, std::shared_ptr<Window> window, Application* app) :
		m_Window{ window }, m_ViewportWidth(screenWidth), m_ViewportHeight(screenHeight), m_App{app}
	{
		
		// create instance
		CreateInstance();

		CreateDebugMessenger();

		CreateSurface();

		CreateDevice();
		RenderContext::SetDevice(m_Device);

		SwapChainSupportDetails support;
		support.devcie = m_Device;
		support.surface = &m_Surface;
		support.capabilities = m_Device->GetDevicePhys().getSurfaceCapabilitiesKHR(m_Surface);
		support.formats = m_Device->GetDevicePhys().getSurfaceFormatsKHR(m_Surface);
		support.presentModes = m_Device->GetDevicePhys().getSurfacePresentModesKHR(m_Surface);
		support.viewportWidth = m_ViewportWidth;
		support.viewportHeight = m_ViewportHeight;

		m_Swapchain = Swapchain::Create(support);

#if 1
		//Vertex{ {-0.5f, -0.5f,0.},{1.0f, 0.0f, 0.0f,1.0f} };
		//Vertex{ {0.5f, -0.5f,0. }, { 0.0f, 1.0f, 0.0f,1.0f }};
		//Vertex{ {0.5f, 0.5f,0.},{0.0f, 0.0f, 1.0f,1.0f} };
		//Vertex{ {-0.5f, 0.5f,0.},{1.0f, 1.0f, 1.0f,1.0f } };

		constexpr size_t VertexCount = 8;

		std::array<Vertex, VertexCount> vertices;
		
		/*vertices[0] = { -1, -1, -1,
						1.0,1.0,1.0,1.0,
						0.0,1.0};
		vertices[1] = { 1, -1, -1,
						1.0,1.0,1.0,1.0, 
						1.0,1.0 };
		vertices[2] = { 1, 1, -1,
						1.0,1.0,1.0,1.0,
						1.0,0.0 };

		vertices[3] = { -1, 1, -1 ,
						1.0,1.0,1.0,1.0,
						0.0,0.0 };
		vertices[4] = { -1, -1, 1
						,1.0,1.0,1.0,1.0,
						0.0,0.0 };
		vertices[5] = { 1, -1, 1 ,
						1.0,1.0,1.0,1.0,
						0.0,0.0 };
		vertices[6] = { 1, 1, 1 ,
						1.0,1.0,1.0,1.0,
						0.0,0.0 };
		vertices[7] = { -1, 1, 1 ,
						1.0,1.0,1.0,1.0,
						0.0,0.0 };*/
		vertices[0] = { -1, -1, -1, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0 };
		vertices[1] = { 1, -1, -1, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0 };
		vertices[2] = { 1, 1, -1, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0 };
		vertices[3] = { -1, 1, -1, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0 };
		vertices[4] = { -1, -1, 1, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0 };
		vertices[5] = { 1, -1, 1, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0 };
		vertices[6] = { 1, 1, 1, 0.5, 0.5, 0.5, 1.0, 0.0, 0.0 };
		vertices[7] = { -1, 1, 1, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0 };

		//const std::vector<uint32_t> indices =
		//{
		//0, 1, 2, 2, 3, 0
		//};

		const std::vector<uint32_t> indices =
		{
			0, 1, 3, 3, 1, 2,
			1, 5, 2, 2, 5, 6,
			5, 4, 6, 6, 4, 7,
			4, 0, 7, 7, 0, 3,
			3, 2, 7, 7, 2, 6,
			4, 5, 0, 0, 5, 1
		};

		
	
#else
		constexpr size_t VertexCount = 36;

		std::array<Vertex, VertexCount> vertices;
		// Front face
		vertices[0] = { -1.0f, -1.0f, 1.0f,1.0,1.0,1.0,1.0 };
		vertices[1] = { 1.0f, -1.0f, 1.0f,1.0,1.0,1.0,1.0 };
		vertices[2] = { 1.0f, 1.0f, 1.0f,1.0,1.0,1.0,1.0 };

		vertices[3] = { -1.0f, -1.0f, 1.0f ,1.0,1.0,1.0,1.0 };
		vertices[4] = { 1.0f, 1.0f, 1.0f ,1.0,1.0,1.0,1.0 };
		vertices[5] = { -1.0f, 1.0f, 1.0f ,1.0,1.0,1.0,1.0 };

		// Back face
		vertices[6] = { -1.0f, -1.0f, -1.0f,1.0,1.0,1.0,1.0 };
		vertices[7] = { -1.0f, 1.0f, -1.0f,1.0,1.0,1.0,1.0 };
		vertices[8] = { 1.0f, 1.0f, -1.0f ,1.0,1.0,1.0,1.0 };

		vertices[9] = { -1.0f, -1.0f, -1.0f,1.0,1.0,1.0,1.0 };
		vertices[10] = { 1.0f, 1.0f, -1.0f,1.0,1.0,1.0,1.0 };
		vertices[11] = { 1.0f, -1.0f, -1.0f ,1.0,1.0,1.0,1.0 };

		// Top face
		vertices[12] = { -1.0f, 1.0f, -1.0f,1.0,1.0,1.0,1.0 };
		vertices[13] = { -1.0f, 1.0f, 1.0f,1.0,1.0,1.0,1.0 };
		vertices[14] = { 1.0f, 1.0f, 1.0f ,1.0,1.0,1.0,1.0 };

		vertices[15] = { -1.0f, 1.0f, -1.0f,1.0,1.0,1.0,1.0 };
		vertices[16] = { 1.0f, 1.0f, 1.0f,1.0,1.0,1.0,1.0 };
		vertices[17] = { 1.0f, 1.0f, -1.0f ,1.0,1.0,1.0,1.0 };

		// Bottom face
		vertices[18] = { -1.0f, -1.0f, -1.0f,1.0,1.0,1.0,1.0 };
		vertices[19] = { 1.0f, -1.0f, -1.0f,1.0,1.0,1.0,1.0 };
		vertices[20] = { 1.0f, -1.0f, 1.0f,1.0,1.0,1.0,1.0 };

		vertices[21] = { -1.0f, -1.0f, -1.0f,1.0,1.0,1.0,1.0 };
		vertices[22] = { 1.0f, -1.0f, 1.0f,1.0,1.0,1.0,1.0 };
		vertices[23] = { -1.0f, -1.0f, 1.0f,1.0,1.0,1.0,1.0 };

		// Left face
		vertices[24] = { -1.0f, -1.0f, -1.0f,1.0,1.0,1.0,1.0 };
		vertices[25] = { -1.0f, -1.0f, 1.0f,1.0,1.0,1.0,1.0 };
		vertices[26] = { -1.0f, 1.0f, 1.0f,1.0,1.0,1.0,1.0 };

		vertices[27] = { -1.0f, -1.0f, -1.0f,1.0,1.0,1.0,1.0 };
		vertices[28] = { -1.0f, 1.0f, 1.0f,1.0,1.0,1.0,1.0 };
		vertices[29] = { -1.0f, 1.0f, -1.0f,1.0,1.0,1.0,1.0 };

		// Right face
		vertices[30] = { 1.0f, -1.0f, 1.0f,1.0,1.0,1.0,1.0 };
		vertices[31] = { 1.0f, -1.0f, -1.0f,1.0,1.0,1.0,1.0 };
		vertices[32] = { 1.0f, 1.0f, -1.0f,1.0,1.0,1.0,1.0 };

		vertices[33] = { 1.0f, -1.0f, 1.0f,1.0,1.0,1.0,1.0 };
		vertices[34] = { 1.0f, 1.0f, -1.0f,1.0,1.0,1.0,1.0 };
		vertices[35] = { 1.0f, 1.0f, 1.0f,1.0,1.0,1.0,1.0 };


	

#endif 

		
		
		 
	

		// create uniform buffers for each frame
		DescrriptorSetLayoutSpec inputLayout;
		inputLayout.type = vk::DescriptorType::eUniformBuffer;
		inputLayout.stages = vk::ShaderStageFlagBits::eVertex;
		
		m_DescriptorSetLayout = DescriptorSetLayout::Create(inputLayout);

		auto bufferSize = sizeof(UniformBufferObject);
		auto framesAmount = m_Swapchain->GetFramesCount();
		m_UniformBuffers.resize(framesAmount);
		uniformBuffersMapped.resize(framesAmount);


		BufferInputChunk inputBuffer;
		inputBuffer.size = bufferSize;
		inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		inputBuffer.usage = vk::BufferUsageFlagBits::eUniformBuffer;

		for (size_t i = 0; i < framesAmount; i++)
		{
			m_UniformBuffers[i] = new Buffer(inputBuffer);
			uniformBuffersMapped[i] = m_Device->GetDevice().mapMemory(m_UniformBuffers[i]->GetMemory(), 0, bufferSize);
	
		}


		m_DescriptorPool = DescriptorPool::Create(vk::DescriptorType::eUniformBuffer, static_cast<uint32_t>(m_Swapchain->GetFramesCount()), m_Swapchain->GetFramesCount());
		

		std::vector<vk::DescriptorSetLayout> layouts(m_Swapchain->GetFramesCount(), m_DescriptorSetLayout->GetLayout());

		
			m_DescriptorSets =  m_DescriptorPool->AllocateDescriptorSets(m_Swapchain->GetFramesCount(), layouts.data());
			

			
		
			


		for (int i = 0; i < m_Swapchain->GetFramesCount(); i++)
		{
			vk::WriteDescriptorSet writeInfo;
			
			vk::DescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_UniformBuffers[i]->GetBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			writeInfo.dstSet = m_DescriptorSets[i];
			writeInfo.dstBinding = 0;
			writeInfo.dstArrayElement = 0; //byte offset within binding for inline uniform blocks
			writeInfo.descriptorCount = 1;
			writeInfo.descriptorType = vk::DescriptorType::eUniformBuffer;
			writeInfo.pBufferInfo = &bufferInfo;

			m_Device->GetDevice().updateDescriptorSets(writeInfo, nullptr);
		}
		

		{


			m_DescriptorPoolTex = DescriptorPool::Create(vk::DescriptorType::eCombinedImageSampler,1,1);

			std::vector<vk::DescriptorSetLayout> layoutBindings;
			layoutBindings.resize(1);
			
			DescrriptorSetLayoutSpec layout;
			layout.type = vk::DescriptorType::eCombinedImageSampler;
			layout.stages = vk::ShaderStageFlagBits::eFragment;

			m_DescriptorSetLayoutTex = DescriptorSetLayout::Create(layout);

			layoutBindings[0] = m_DescriptorSetLayoutTex->GetLayout();

			m_DescriptorSetTex = m_DescriptorPoolTex->AllocateDescriptorSets(1, layoutBindings.data())[0];
		}

		m_GraphicsQueue = CreateUPtr<Queue>();
		m_GraphicsQueue->CreateCommandPool();
		m_GraphicsQueue->CreateCommandBuffer();
		RenderContext::SetGraphicsQueue(m_GraphicsQueue.get());
		CreateSyncObjects();

		m_Image = Image::CreateImage("res/viking_room/viking_room.png", m_DescriptorSetTex);
		std::string modelPath = "res/viking_room/viking_room.obj";
		m_Model = Model::Load(modelPath);





		CreatePipeline();
		CreateFramebuffers();
		



		auto indexSize = m_Model->indices.size() * sizeof(m_Model->indices[0]);
		{
			SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(indexSize);
		

			{
				BufferInputChunk inputBuffer;
				inputBuffer.size = indexSize;
				inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
				inputBuffer.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
				m_IndexBuffer = new IndexBuffer(inputBuffer, m_Model->indices.size(),vk::IndexType::eUint32);

			}

			m_GraphicsQueue->BeginTransfering();
			m_GraphicsQueue->Transfer(stagingBuffer.get(), m_IndexBuffer, (void*)m_Model->indices.data(), indexSize);
			m_GraphicsQueue->EndTransfering();
			
		
			
		}

		auto vertexSize = m_Model->vertices.size() * sizeof(Vertex);
		void* vertexData = const_cast<void*>(static_cast<const void*>(m_Model->vertices.data()));
		SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(vertexSize);
		{
			BufferInputChunk inputBuffer;
			inputBuffer.size = vertexSize;
			inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
			inputBuffer.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;

			m_Buffer = new Buffer(inputBuffer);
		}

		m_GraphicsQueue->BeginTransfering();
		m_GraphicsQueue->Transfer(stagingBuffer.get(), m_Buffer, (void*)m_Model->vertices.data(), vertexSize);
		m_GraphicsQueue->EndTransfering();
		



		


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


	void Renderer::UpdateUniformBuffer(uint32_t imageIndex)
	{
		UniformBufferObject ubo{};
		
		auto cameraView = m_App->GetCamera()->GetView();
		auto cameraProj = m_App->GetCamera()->GetProj();

		ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(-2.0f, 0.0f, 4.0f), glm::vec3(0.5f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		//ubo.proj = glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.0001f, 10.0f);
		ubo.view = cameraView;
		ubo.proj = cameraProj;
		auto extent = m_Swapchain->GetExtent();
		//ubo.proj[1][1] *= -1,
		memcpy(uniformBuffersMapped[imageIndex], &ubo, sizeof(ubo));
	}

	void Renderer::RecordCommandBuffer(uint32_t imageIndex)
	{
		
		m_GraphicsQueue->BeginRenderPass(&m_RenderPass, &m_Swapchain->GetFrames()[imageIndex].framebuffer, &m_Swapchain->GetExtent());


		m_GraphicsQueue->RecordCommand(m_Buffer, m_IndexBuffer, &m_Pipeline, &m_PipelineLayout, &m_DescriptorSets[imageIndex],&m_DescriptorSetTex);
			

		m_GraphicsQueue->EndRenderPass();
		
	}
	void Renderer::Render()	
	{
		
		m_Device->GetDevice().waitForFences(m_InFlightFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
		m_Device->GetDevice().resetFences(m_InFlightFence);

		uint32_t imageIndex;
		auto swapchain = m_Swapchain->GetSwapChain();
		m_Device->GetDevice().acquireNextImageKHR(swapchain, UINT64_MAX, m_ImageAvailableSemaphore, nullptr, &imageIndex);
		
	
		m_GraphicsQueue->BeginRendering();


		UpdateUniformBuffer(imageIndex);

		RecordCommandBuffer(imageIndex);
		vk::Semaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
		vk::Semaphore signalSemaphores[] = { m_RenderFinishedSemaphore };
		m_GraphicsQueue->Submit(waitSemaphores, signalSemaphores, &m_InFlightFence);
		m_GraphicsQueue->EndRendering();

		

		
		

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

	
	void Renderer::CreateFramebuffers()
	{
		auto& frames = m_Swapchain->GetFrames();
		auto swapChainExtent = m_Swapchain->GetExtent();
		for (int i = 0; i < frames.size(); ++i) {

			std::vector<vk::ImageView> attachments = {
				frames[i].imageView,
				frames[i].imageDepthView,
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

		m_DescriptorSetLayouts = std::vector<vk::DescriptorSetLayout>{ m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutTex->GetLayout()};

		specs.descriptorSetLayout = m_DescriptorSetLayouts;
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

	vk::RenderPass MakeRenderPass(vk::Device device, vk::Format swapchainImageFormat, vk::Format depthFormat) {

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


		vk::AttachmentDescription depthAttachment = {};
		depthAttachment.flags = vk::AttachmentDescriptionFlags();
		depthAttachment.format = depthFormat;
		depthAttachment.samples = vk::SampleCountFlagBits::e1;
		depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
		depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::AttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		//Renderpasses are broken down into subpasses, there's always at least one.
		vk::SubpassDescription subpass = {};
		subpass.flags = vk::SubpassDescriptionFlags();
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment= &depthAttachmentRef;


		vk::SubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		// define order of subpasses?
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.srcAccessMask = vk::AccessFlagBits::eNone;
		// define rights of subpasses?
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput| vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		//Now create the renderpass
		vk::RenderPassCreateInfo renderpassInfo = {};

		std::vector<vk::AttachmentDescription> attachments = { colorAttachment ,depthAttachment };
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
		
		vk::RenderPass renderpass = MakeRenderPass(spec.device, spec.swapchainImageFormat,m_Swapchain->GetFrames()[0].depthFormat);
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
		
		delete m_Buffer;
		delete m_IndexBuffer;
		m_Image.reset();



		m_GraphicsQueue.reset();

		for (size_t i = 0; i < m_Swapchain->GetFramesCount(); i++) {
			delete m_UniformBuffers[i];
		}
		
		delete m_DescriptorSetLayout;
		delete m_DescriptorSetLayoutTex;
		m_DescriptorPool.reset();
		m_DescriptorPoolTex.reset();
		
	

		for (int i = 0; i < m_DescriptorSetLayouts.size(); i++)
		{
			m_Device->GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayouts[i]);
		}
	
	
		
		m_Device->GetDevice().destroySemaphore(m_ImageAvailableSemaphore);
		m_Device->GetDevice().destroySemaphore(m_RenderFinishedSemaphore);
		m_Device->GetDevice().destroyFence(m_InFlightFence);

		m_Device->GetDevice().destroyPipeline(m_Pipeline);
		m_Device->GetDevice().destroyPipelineLayout(m_PipelineLayout);
		delete m_Swapchain;
		m_Device->GetDevice().destroyRenderPass(m_RenderPass);

		m_Device->GetDevice().destroy();

		m_Instance->GetInstance().destroySurfaceKHR(m_Surface);
		m_Instance->GetInstance().destroyDebugUtilsMessengerEXT(m_DebugMessenger, nullptr, m_Dldi);
		m_Instance->GetInstance().destroy();
	}
	void Renderer::CreateDevice()
	{
		m_Device = Device::Create(m_Instance, m_Surface);
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
