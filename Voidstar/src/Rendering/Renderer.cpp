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
#include "RenderContext.h"
#include "DescriptorSetLayout.h"
#include "Camera.h"
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

		constexpr size_t VertexCount = 4;

		std::array<Vertex, VertexCount> vertices;
		
		vertices[0] = { -0.5f, -0.5f, 1.0f,1.0,1.0,1.0,1.0 };
		vertices[1] = { 0.5f, -0.5f, 1.0f,1.0,1.0,1.0,1.0 };
		vertices[2] = { 0.5f, 0.5f, 1.0f,1.0,1.0,1.0,1.0 };

		vertices[3] = { -0.5f, 0.5f, 1.0f ,1.0,1.0,1.0,1.0 };
		//vertices[4] = { 1.0f, 1.0f, 1.0f ,1.0,1.0,1.0,1.0 };
		//vertices[5] = { -1.0f, 1.0f, 1.0f ,1.0,1.0,1.0,1.0 };
		const std::vector<uint32_t> indices =
		{
		0, 1, 2, 2, 3, 0
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
		inputLayout.flags = vk::ShaderStageFlagBits::eVertex;
		
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

		vk::DescriptorPoolSize poolSize{};
		poolSize.type = vk::DescriptorType::eUniformBuffer;
		poolSize.descriptorCount = static_cast<uint32_t>(m_Swapchain->GetFramesCount());
		
		vk::DescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = vk::StructureType::eDescriptorPoolCreateInfo;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = m_Swapchain->GetFramesCount();

		try
		{
			m_DescriptorPool = m_Device->GetDevice().createDescriptorPool(poolInfo);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("failed to create descriptorPool");
		}

		std::vector<vk::DescriptorSetLayout> layouts(m_Swapchain->GetFramesCount(), m_DescriptorSetLayout->GetLayout());

		//for (int i = 0; i < m_Swapchain->GetFramesCount(); i++)
		//{
			vk::DescriptorSetAllocateInfo allocationInfo;
			/*
				typedef struct VkDescriptorSetAllocateInfo {
					VkStructureType                 sType;
					const void*                     pNext;
					VkDescriptorPool                descriptorPool;
					uint32_t                        descriptorSetCount;
					const VkDescriptorSetLayout*    pSetLayouts;
				} VkDescriptorSetAllocateInfo;
			*/

			allocationInfo.descriptorPool = m_DescriptorPool;
			allocationInfo.descriptorSetCount = m_Swapchain->GetFramesCount();
			allocationInfo.pSetLayouts = layouts.data();

			//m_DescriptorSets.resize(m_Swapchain->GetFramesCount());
			
			try 
			{
				m_DescriptorSets = m_Device->GetDevice().allocateDescriptorSets(allocationInfo);
			}
			catch (vk::SystemError err)
			{	
				Log::GetLog()->error("Failed to allocate descriptor set from pool");
			}	
		//}

		for (int i = 0; i < m_Swapchain->GetFramesCount(); i++)
		{
			vk::WriteDescriptorSet writeInfo;
			/*
			typedef struct VkWriteDescriptorSet {
				VkStructureType                  sType;
				const void* pNext;
				VkDescriptorSet                  dstSet;
				uint32_t                         dstBinding;
				uint32_t                         dstArrayElement;
				uint32_t                         descriptorCount;
				VkDescriptorType                 descriptorType;
				const VkDescriptorImageInfo* pImageInfo;
				const VkDescriptorBufferInfo* pBufferInfo;
				const VkBufferView* pTexelBufferView;
			} VkWriteDescriptorSet;
			*/
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
		
		
		CreatePipeline();


		CreateFramebuffers();

		CreateCommandPool();
		CreateCommandBuffer();
		CreateSyncObjects();

		{
			Buffer* stagingBuffer;
			{

				BufferInputChunk inputBuffer;
				inputBuffer.size = sizeof(indices);
				inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
				inputBuffer.usage = vk::BufferUsageFlagBits::eTransferSrc;

				stagingBuffer = new Buffer(inputBuffer);
			}

			{
				BufferInputChunk inputBuffer;
				inputBuffer.size = sizeof(indices);
				inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
				inputBuffer.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
				m_IndexBuffer = new Buffer(inputBuffer);

			}

			m_CommandBuffer.reset();

			vk::CommandBufferBeginInfo beginInfo;
			beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
			m_CommandBuffer.begin(beginInfo);


			auto memory = m_Device->GetDevice().mapMemory(stagingBuffer->GetMemory(), 0, sizeof(indices));
			memcpy(memory, indices.data(), (size_t)sizeof(indices));
			m_Device->GetDevice().unmapMemory(stagingBuffer->GetMemory());


			vk::BufferCopy copyRegion{};
			copyRegion.srcOffset = 0; // Optional
			copyRegion.dstOffset = 0; // Optional
			copyRegion.size = sizeof(indices);
			m_CommandBuffer.copyBuffer(stagingBuffer->GetBuffer(), m_IndexBuffer->GetBuffer(), copyRegion);


			m_CommandBuffer.end();

			vk::SubmitInfo submitInfo;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &m_CommandBuffer;
			m_Device->GetGraphicsQueue().submit(1, &submitInfo, nullptr);

			m_Device->GetGraphicsQueue().waitIdle();

			delete stagingBuffer;
		}


		void* vertexData = const_cast<void*>(static_cast<const void*>(vertices.data()));
		//m_Buffer->SetData(vertexData);
		Buffer* stagingBuffer;
		{

			BufferInputChunk inputBuffer;
			inputBuffer.size = sizeof(vertices);
			inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			inputBuffer.usage = vk::BufferUsageFlagBits::eTransferSrc;

			stagingBuffer = new Buffer(inputBuffer);
		}

		{
			BufferInputChunk inputBuffer;
			inputBuffer.size = sizeof(vertices);
			inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
			inputBuffer.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;

			m_Buffer = new Buffer(inputBuffer);
		}

		m_CommandBuffer.reset();

		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		m_CommandBuffer.begin(beginInfo);


		bufferSize = sizeof(vertices);
		auto memory = m_Device->GetDevice().mapMemory(stagingBuffer->GetMemory(), 0, bufferSize);
		memcpy(memory, vertexData, (size_t)bufferSize);
		m_Device->GetDevice().unmapMemory(stagingBuffer->GetMemory());


		vk::BufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = sizeof(vertices);
		m_CommandBuffer.copyBuffer(stagingBuffer->GetBuffer(), m_Buffer->GetBuffer(), copyRegion);


		m_CommandBuffer.end();

		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;
		m_Device->GetGraphicsQueue().submit(1, &submitInfo, nullptr);

		m_Device->GetGraphicsQueue().waitIdle();


		delete stagingBuffer;

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

		m_CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout, 0, m_DescriptorSets[imageIndex], nullptr);
		m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
		vk::DeviceSize offsets[] = { 0 };
		vk::Buffer vertexBuffers[] = { m_Buffer->GetBuffer()};
		m_CommandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		m_CommandBuffer.bindIndexBuffer(m_IndexBuffer->GetBuffer(),0, vk::IndexType::eUint32);
		//m_CommandBuffer.draw(6, 1, 0, 0);
		//m_CommandBuffer.draw(23, 1, 0, 0);
		m_CommandBuffer.drawIndexed(6,1,0,0,0);

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

		UpdateUniformBuffer(imageIndex);

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

		m_DescriptorSetLayouts = std::vector<vk::DescriptorSetLayout>{ m_DescriptorSetLayout->GetLayout() };

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
		
		delete m_Buffer;
		delete m_IndexBuffer;


		for (size_t i = 0; i < m_Swapchain->GetFramesCount(); i++) {
			delete m_UniformBuffers[i];
		}
		
		delete m_DescriptorSetLayout;
		m_Device->GetDevice().destroyDescriptorPool(m_DescriptorPool);

		for (int i = 0; i < m_DescriptorSetLayouts.size(); i++)
		{
			m_Device->GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayouts[i]);
		}
	
	
		m_Device->GetDevice().destroyCommandPool(m_CommandPool);
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
