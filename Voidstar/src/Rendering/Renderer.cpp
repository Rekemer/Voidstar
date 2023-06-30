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
#include <random>
#include"tracy/Tracy.hpp"
#include"tracy/TracyVulkan.hpp"
namespace Voidstar
{
	TracyVkCtx ctx;
	vk::ShaderModule CreateModule(std::string filename, vk::Device device);
	const uint32_t PARTICLE_COUNT = 8192;
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
	
		
		vk::DescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = 0;
		layoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
		layoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
		layoutBinding.descriptorCount = 1;

		std::vector<vk::DescriptorSetLayoutBinding> layoutBindings{ layoutBinding };
		m_DescriptorSetLayout = DescriptorSetLayout::Create(layoutBindings);

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




		{

			vk::DescriptorPoolSize poolSize;
			poolSize.type = vk::DescriptorType::eUniformBuffer;
			poolSize.descriptorCount = static_cast<uint32_t>(m_Swapchain->GetFramesCount());
			
			std::vector<vk::DescriptorPoolSize> poolSizes{ poolSize };
			
			m_DescriptorPool = DescriptorPool::Create(poolSizes, m_Swapchain->GetFramesCount());
		}
		

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

			vk::DescriptorPoolSize poolSize;
			poolSize.type = vk::DescriptorType::eCombinedImageSampler;
			poolSize.descriptorCount = 1;
		
			std::vector<vk::DescriptorPoolSize> poolSizes{poolSize};

			m_DescriptorPoolTex = DescriptorPool::Create(poolSizes,1);

			std::vector<vk::DescriptorSetLayout> layouts;
			layouts.resize(1);
			
		

			vk::DescriptorSetLayoutBinding layoutBinding;
			layoutBinding.binding = 0;
			layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			layoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
			layoutBinding.descriptorCount = 1;

			std::vector<vk::DescriptorSetLayoutBinding> layoutBindings{ layoutBinding };

			m_DescriptorSetLayoutTex = DescriptorSetLayout::Create(layoutBindings);

			layouts[0] = m_DescriptorSetLayoutTex->GetLayout();
			//auto check = m_DescriptorPoolTex->AllocateDescriptorSets(2, layouts.data());
			m_DescriptorSetTex = m_DescriptorPoolTex->AllocateDescriptorSets(1, layouts.data())[0];
		}






		m_GraphicsQueue = CreateUPtr<Queue>();
		m_GraphicsQueue->CreateCommandPool();
		m_GraphicsQueue->CreateCommandBuffer();


		

		RenderContext::SetGraphicsQueue(m_GraphicsQueue.get());
		CreateSyncObjects();



		{
			vk::DescriptorPoolSize poolSize;
			poolSize.type = vk::DescriptorType::eStorageBuffer;
			poolSize.descriptorCount = m_Swapchain->GetFramesCount()  *2;

			std::vector<vk::DescriptorPoolSize> poolSizes{ poolSize };

			m_ComputePool = DescriptorPool::Create(poolSizes, m_Swapchain->GetFramesCount());

			vk::DescriptorSetLayoutBinding layoutBinding;
			layoutBinding.binding = 0;
			layoutBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
			layoutBinding.stageFlags = vk::ShaderStageFlagBits::eCompute;
			layoutBinding.descriptorCount = 1;
			vk::DescriptorSetLayoutBinding layoutBinding1;
			layoutBinding1.binding = 1;
			layoutBinding1.descriptorType = vk::DescriptorType::eStorageBuffer;
			layoutBinding1.stageFlags = vk::ShaderStageFlagBits::eCompute;
			layoutBinding1.descriptorCount =1;

			std::vector<vk::DescriptorSetLayoutBinding> layoutBindings{ layoutBinding,layoutBinding1 };

			m_ComputeSetLayout = DescriptorSetLayout::Create(layoutBindings);

			std::vector<vk::DescriptorSetLayout> layouts(m_Swapchain->GetFramesCount(), m_ComputeSetLayout->GetLayout());
			/*VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_ComputePool->GetPool();
			allocInfo.descriptorSetCount = static_cast<uint32_t>(m_Swapchain->GetFramesCount());
			allocInfo.pSetLayouts = layouts.data();*/

			m_ComputeDescriptorSets = m_ComputePool->AllocateDescriptorSets(m_Swapchain->GetFramesCount(), layouts.data());

			//m_DescriptorSetTex = m_DescriptorPoolTex->AllocateDescriptorSets(1, layouts.data())[0];
			
		}


		std::default_random_engine rndEngine((unsigned)time(nullptr));
		std::uniform_real_distribution<float> rndDist(-1.0f, 1.0f);

		// Initial particle positions on a circle
		std::vector<Particle> particles(PARTICLE_COUNT);
		for (auto& particle : particles) {
			float r = 0.25f * sqrt(rndDist(rndEngine));
			float theta = rndDist(rndEngine) * 2 * 3.14159265358979323846;
			float x = r * cos(theta) * m_ViewportHeight/ m_ViewportWidth;
			float y = r * sin(theta);
			particle.position = glm::vec2(x, y);
			particle.velocity = glm::normalize(glm::vec2(x, y)) * 2.f* rndDist(rndEngine);
			particle.color = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.0f);
		}
		auto size = sizeof(Particle) * PARTICLE_COUNT;
		m_ShaderStorageBuffers.resize(3);
		m_CommandComputeBuffers.resize(3);
		for (size_t i = 0; i < m_Swapchain->GetFramesCount(); i++)
		{
			SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(size);


			{
				BufferInputChunk inputBuffer;
				inputBuffer.size = size;
				inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
				inputBuffer.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer;
				m_ShaderStorageBuffers[i] = CreateUPtr<Buffer>(inputBuffer);
			}
			


			m_GraphicsQueue->BeginTransfering();
			m_GraphicsQueue->Transfer(stagingBuffer.get(), m_ShaderStorageBuffers[i].get(), (void*)particles.data(), size);
			m_GraphicsQueue->EndTransfering();



		}
		auto device = RenderContext::GetDevice();
		vk::CommandPoolCreateInfo poolInfo;
		poolInfo.flags = vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		poolInfo.queueFamilyIndex = device->GetGraphicsIndex();
		try
		{
			m_CommandComputePool = device->GetDevice().createCommandPool(poolInfo);

		}
		catch (vk::SystemError err)
		{

			Log::GetLog()->error("Failed to create Command Pool");
		}


		vk::CommandBufferAllocateInfo allocInfo = {};
		allocInfo.commandPool = m_CommandComputePool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = 3;
		try
		{
			m_CommandComputeBuffers = device->GetDevice().allocateCommandBuffers(allocInfo);

		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to allocate  command buffer ");
		}


		for (size_t i = 0; i < m_Swapchain->GetFramesCount(); i++)
		{
			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			VkDescriptorBufferInfo storageBufferInfoLastFrame{};
			storageBufferInfoLastFrame.buffer = m_ShaderStorageBuffers[(i - 1) % m_Swapchain->GetFramesCount()]->GetBuffer();
			storageBufferInfoLastFrame.offset = 0;
			storageBufferInfoLastFrame.range = sizeof(Particle) * PARTICLE_COUNT;

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = m_ComputeDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &storageBufferInfoLastFrame;

			VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
			storageBufferInfoCurrentFrame.buffer = m_ShaderStorageBuffers[i]->GetBuffer();
			storageBufferInfoCurrentFrame.offset = 0;
			storageBufferInfoCurrentFrame.range = sizeof(Particle) * PARTICLE_COUNT;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = m_ComputeDescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &storageBufferInfoCurrentFrame;

			auto device = RenderContext::GetDevice()->GetDevice();
			vkUpdateDescriptorSets(device, 2, descriptorWrites.data(), 0, nullptr);
		}





		//m_Image = Image::CreateImage("res/viking_room/viking_room.png", m_DescriptorSetTex);
		//std::string modelPath = "res/viking_room/viking_room.obj";
		//m_Model = Model::Load(modelPath);


	/*	auto indexSize = m_Model->indices.size() * sizeof(m_Model->indices[0]);
		{
			SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(indexSize);


			{
				BufferInputChunk inputBuffer;
				inputBuffer.size = indexSize;
				inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
				inputBuffer.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
				m_IndexBuffer = new IndexBuffer(inputBuffer, m_Model->indices.size(), vk::IndexType::eUint32);

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
		m_GraphicsQueue->EndTransfering();*/




		auto computeShaderModule = CreateModule("../Shaders/compute.spv", device->GetDevice());

		vk::PipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.flags = vk::PipelineShaderStageCreateFlags();
		computeShaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
		computeShaderStageInfo.module = computeShaderModule;
		computeShaderStageInfo.pName = "main";


		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = const_cast<const vk::DescriptorSetLayout*>(&m_ComputeSetLayout->GetLayout());
		m_ComputePipelineLayout = device->GetDevice().createPipelineLayout(pipelineLayoutInfo, nullptr);


		vk::ComputePipelineCreateInfo pipelineInfo{};

		pipelineInfo.sType = vk::StructureType::eComputePipelineCreateInfo;
		pipelineInfo.layout = m_ComputePipelineLayout;
		pipelineInfo.stage = computeShaderStageInfo;
		m_ComputePipeline = device->GetDevice().createComputePipeline(nullptr, pipelineInfo).value;

		

		vkDestroyShaderModule(device->GetDevice(), computeShaderModule, nullptr);


		CreateMSAAFrame();
		CreatePipeline();
		CreateFramebuffers();
		


		auto physDev = m_Device->GetDevicePhys();
		auto dev = m_Device->GetDevice();
		auto queue = m_Device->GetGraphicsQueue();
		auto commandBuffer = m_GraphicsQueue->GetCommandBuffer();

		auto instance = m_Instance->GetInstance();
		PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT vkGetPhysicalDeviceCalibrateableTimeDomainsEXT = reinterpret_cast<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT"));
		PFN_vkGetCalibratedTimestampsEXT vkGetCalibratedTimestampsEXT = reinterpret_cast<PFN_vkGetCalibratedTimestampsEXT>(vkGetDeviceProcAddr(dev, "vkGetCalibratedTimestampsEXT"));

		ctx = TracyVkContextCalibrated(physDev,dev,queue,commandBuffer, 
			vkGetPhysicalDeviceCalibrateableTimeDomainsEXT, vkGetCalibratedTimestampsEXT);


		


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
		auto frameAmount = m_Swapchain->GetFramesCount();
		m_ComputeInFlightFences.resize(frameAmount);
		m_ComputeFinishedSemaphores.resize(frameAmount);
		for (int i = 0; i < frameAmount; i++)
		{
			vk::SemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.flags = vk::SemaphoreCreateFlags();
			m_ComputeFinishedSemaphores[i] = m_Device->GetDevice().createSemaphore(semaphoreInfo);

			vk::FenceCreateInfo fenceInfo = {};
			fenceInfo.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;

			m_ComputeInFlightFences[i] = m_Device->GetDevice().createFence(fenceInfo);
		}
	
	}

	void Renderer::CreateMSAAFrame()
	{
		ImageSpecs specs;
		auto extent = m_Swapchain->GetExtent();
		auto swapchainFormat = m_Swapchain->GetFormat();
		specs.width = extent.width;
		specs.height= extent.height;
		specs.tiling = vk::ImageTiling::eOptimal;
		specs.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment;
		specs.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		specs.format= swapchainFormat;
		auto samples = RenderContext::GetDevice()->GetSamples();
		m_MsaaImage = Image::CreateVKImage(specs,samples);
		m_MsaaImageMemory = Image::CreateMemory(m_MsaaImage,specs);
		m_MsaaImageView = Image::CreateImageView(m_MsaaImage,swapchainFormat,vk::ImageAspectFlagBits::eColor);
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
		auto model = glm::mat4(1.f);
		glm::mat4 blenderToLH = glm::mat4(1.0f);
		blenderToLH[2][2] = -1.0f;  // Flip Z-axis
		blenderToLH[3][2] = 1.0f;
		//model = blenderToLH * model;
		// blender: z  is up, y is forward
		model = glm::rotate(model,glm::radians(-90.f) , glm::vec3(1, 0, 0));
		model = glm::rotate(model,glm::radians(90.f) , glm::vec3(0, 0, 1));
		ubo.model = model;
		auto extent = m_Swapchain->GetExtent();
		//ubo.proj[1][1] *= -1,
		memcpy(uniformBuffersMapped[imageIndex], &ubo, sizeof(ubo));
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
		m_Swapchain.reset();

		m_Device->GetDevice().freeMemory(m_MsaaImageMemory);
		m_Device->GetDevice().destroyImage(m_MsaaImage);
		m_Device->GetDevice().destroyImageView(m_MsaaImageView);

		SwapChainSupportDetails support;
		support.devcie = m_Device;
		support.surface = &m_Surface;
		support.capabilities = m_Device->GetDevicePhys().getSurfaceCapabilitiesKHR(m_Surface);
		support.formats = m_Device->GetDevicePhys().getSurfaceFormatsKHR(m_Surface);
		support.presentModes = m_Device->GetDevicePhys().getSurfacePresentModesKHR(m_Surface);
		support.viewportWidth = m_ViewportWidth;
		support.viewportHeight = m_ViewportHeight;
		m_Swapchain = Swapchain::Create(support);
		CreateMSAAFrame();



		CreateFramebuffers();
		auto& camera = m_App->GetCamera();
		camera->UpdateProj(m_ViewportWidth, m_ViewportHeight);
	}


	void Renderer::RecordCommandBuffer(uint32_t imageIndex)
	{

		vk::CommandBufferBeginInfo beginInfo = {};

		auto commandBuffer = m_GraphicsQueue->GetCommandBuffer();

		commandBuffer.begin(beginInfo);
		{
			TracyVkZone(ctx, commandBuffer, "Rendering");
		m_GraphicsQueue->BeginRenderPass(&m_RenderPass, &m_Swapchain->GetFrames()[imageIndex].framebuffer, &m_Swapchain->GetExtent());
		vk::Viewport viewport;
		viewport.x = 0;
		viewport.y = 0;
		viewport.minDepth = 0;
		viewport.maxDepth = 1;
		viewport.height = m_ViewportHeight;
		viewport.width = m_ViewportWidth;

		vk::Rect2D scissors;
		scissors.offset = vk::Offset2D{(uint32_t)0,(uint32_t)0};
		scissors.extent= vk::Extent2D{ (uint32_t)m_ViewportWidth,(uint32_t)m_ViewportHeight};
		m_GraphicsQueue->RecordCommand(m_ShaderStorageBuffers[imageIndex].get(), m_IndexBuffer, &m_Pipeline, &m_PipelineLayout, &m_DescriptorSets[imageIndex], &m_DescriptorSetTex, viewport, scissors);
		}


			m_GraphicsQueue->EndRenderPass();
			TracyVkCollect(ctx, commandBuffer);
		commandBuffer.end();
		
	}

	size_t currentFrame = 0;
	void Renderer::Render()	
	{
		

		{
			ZoneScopedN("Waiting for fence ");
			
		m_Device->GetDevice().waitForFences(m_ComputeInFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
		}
		m_Device->GetDevice().resetFences(m_ComputeInFlightFences[currentFrame]);




		m_CommandComputeBuffers[currentFrame].reset();


		{

			vk::CommandBufferBeginInfo beginInfo;
			beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

			

			m_CommandComputeBuffers[currentFrame].begin(beginInfo);
			TracyVkZone(ctx, m_CommandComputeBuffers[currentFrame], "Compute");
			vkCmdBindPipeline(m_CommandComputeBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE,	m_ComputePipeline);
			auto descSet = const_cast<const vk::DescriptorSet*>(&m_ComputeDescriptorSets[currentFrame]);
			m_CommandComputeBuffers[currentFrame].bindDescriptorSets(vk::PipelineBindPoint::eCompute,	m_ComputePipelineLayout, 0, 1, descSet, 0, 0);

			vkCmdDispatch(m_CommandComputeBuffers[currentFrame], PARTICLE_COUNT / 256, 1, 1);
		}
		TracyVkCollect(ctx, m_CommandComputeBuffers[currentFrame]);
			if (vkEndCommandBuffer(m_CommandComputeBuffers[currentFrame]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to record command buffer!");
			}
			



		
		vk::SubmitInfo submitInfo = {};

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandComputeBuffers[currentFrame];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_ComputeFinishedSemaphores[currentFrame];

		m_Device->GetGraphicsQueue().submit(submitInfo, m_ComputeInFlightFences[currentFrame]);

		{

			ZoneScopedN("Waiting for fence");
			m_Device->GetDevice().waitForFences(m_InFlightFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
		}
		m_Device->GetDevice().resetFences(m_InFlightFence);

		uint32_t imageIndex;
		auto swapchain = m_Swapchain->GetSwapChain();
		{
			ZoneScopedN("Acquiring new Image");
			m_Device->GetDevice().acquireNextImageKHR(swapchain, UINT64_MAX, m_ImageAvailableSemaphore, nullptr, &imageIndex);
		}
		{
			ZoneScopedN("Updating uniform buffer");

			UpdateUniformBuffer(imageIndex);
		}
		
	

		
			vk::Semaphore waitSemaphores[] = { m_ImageAvailableSemaphore,m_ComputeFinishedSemaphores[currentFrame] };
			vk::Semaphore signalSemaphores[] = { m_RenderFinishedSemaphore };

		{
			ZoneScopedN("Sumbit render commands");
			m_GraphicsQueue->BeginRendering();
			RecordCommandBuffer(imageIndex);
			m_GraphicsQueue->Submit(waitSemaphores, signalSemaphores, &m_InFlightFence);
			m_GraphicsQueue->EndRendering();
		}

		

		

		

		vk::PresentInfoKHR presentInfo = {};
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		vk::SwapchainKHR swapChains[] = { swapchain };
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

		currentFrame = (currentFrame + 1) % m_Swapchain->GetFramesCount();


		
		FrameMark;
	}

	
	void Renderer::CreateFramebuffers()
	{
		auto& frames = m_Swapchain->GetFrames();
		auto swapChainExtent = m_Swapchain->GetExtent();
		for (int i = 0; i < frames.size(); ++i) {

			std::vector<vk::ImageView> attachments = {
				m_MsaaImageView,
				frames[i].imageDepthView,
				frames[i].imageView,
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
		specs.bindingDescription = Particle::GetBindingDescription();
		//specs.attributeDescription = Vertex::GetAttributeDescriptions();
		specs.attributeDescription = Particle::GetAttributeDescriptions();

		auto samples = RenderContext::GetDevice()->GetSamples();
		specs.samples = samples;
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
		
		
		vk::AttachmentReference msaaAttachmentRef = {};
		msaaAttachmentRef.attachment = 0;
		msaaAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
		


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
		colorAttachmentResolve.finalLayout = vk::ImageLayout::ePresentSrcKHR;

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
		subpass.pDepthStencilAttachment= &depthAttachmentRef;
		subpass.pResolveAttachments= &colorAttachmentRef;


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

		std::vector<vk::AttachmentDescription> attachments = {  msaaAttachment,depthAttachment,colorAttachmentResolve };
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
		inputAssemblyInfo.topology = vk::PrimitiveTopology::ePointList;
		
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

		auto device = m_Device->GetDevice();
		device.waitIdle();
		
		TracyVkDestroy(ctx)
		if (m_Buffer != nullptr)
		{
			delete m_Buffer;
			delete m_IndexBuffer;

		}
		m_Image.reset();
		
		m_Device->GetDevice().freeMemory(m_MsaaImageMemory);
		m_Device->GetDevice().destroyImage(m_MsaaImage);
		m_Device->GetDevice().destroyImageView(m_MsaaImageView);

		m_GraphicsQueue.reset();

		for (size_t i = 0; i < m_Swapchain->GetFramesCount(); i++) {
			delete m_UniformBuffers[i];
		}
		for (auto& buffer : m_ShaderStorageBuffers)
		{
			buffer.reset();
		}
		device.destroyCommandPool(m_CommandComputePool);
		
		delete m_DescriptorSetLayout;
		delete m_DescriptorSetLayoutTex;
		m_DescriptorPool.reset();
		m_DescriptorPoolTex.reset();
		m_ComputePool.reset();
		m_Device->GetDevice().destroyDescriptorSetLayout(m_ComputeSetLayout->GetLayout());
	

		for (int i = 0; i < m_DescriptorSetLayouts.size(); i++)
		{
			m_Device->GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayouts[i]);
		}
		
	
		
		m_Device->GetDevice().destroySemaphore(m_ImageAvailableSemaphore);
		m_Device->GetDevice().destroySemaphore(m_RenderFinishedSemaphore);
		m_Device->GetDevice().destroyFence(m_InFlightFence);

		for (auto& semaphore : m_ComputeFinishedSemaphores)
		{
			m_Device->GetDevice().destroySemaphore(semaphore);
		}
		for (auto& fence : m_ComputeInFlightFences)
		{
			m_Device->GetDevice().destroyFence(fence);
		}
		m_Device->GetDevice().destroyPipeline(m_Pipeline);
		m_Device->GetDevice().destroyPipelineLayout(m_PipelineLayout);
		m_Swapchain.reset();
		m_Device->GetDevice().destroyRenderPass(m_RenderPass);

		m_Device->GetDevice().destroyPipeline(m_ComputePipeline);
		m_Device->GetDevice().destroyPipelineLayout(m_ComputePipelineLayout);


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
		//tracy
		info.extensions.push_back("VK_EXT_calibrated_timestamps");

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
