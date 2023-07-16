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
#include "CommandPoolManager.h"
#include"tracy/Tracy.hpp"
#include"tracy/TracyVulkan.hpp"
#include <filesystem>
#include <cstdlib>
#include <algorithm>


namespace std
{
	template<>
	struct hash<glm::vec3>
	{
		size_t operator()(const glm::vec3& key) const
		{
			std::string keyString = std::to_string(key.x) + std::to_string(key.y) + std::to_string(key.z);
			return std::hash<std::string>()(keyString);
		}
	};
}
namespace Voidstar
{
	

	TracyVkCtx ctx;
	vk::ShaderModule CreateModule(std::string filename, vk::Device device);
	const uint32_t PARTICLE_COUNT = 8192;
	std::string BASE_SHADER_PATH = "../Shaders/";
	std::string BASE_RES_PATH = "res";
	//const std::string SPIRV_COMPILER_PATH = "C:/VulkanSDK/1.3.216.0/Bin/glslc.exe";
	const std::string SPIRV_COMPILER_PATH = "C:/VulkanSDK/1.3.216.0/Bin/glslangvalidator.exe";
	std::string BASE_SPIRV_OUTPUT = BASE_SHADER_PATH+"Binary/";
	#define INSTANCE_COUNT 4096
	#define ZEROPOS 1
	

	template<typename T>
	const uint64_t SizeOfBuffer(const uint64_t bufferSize,const T& bufferElement) 
	{
		return bufferSize * sizeof(bufferElement);
	}
	std::string GetFileNameWithoutExtension(const std::string& filepath)
	{
		size_t extensionIndex = filepath.find_last_of('.');
		return filepath.substr(0, extensionIndex);
	}

	std::string CreateCommand(std::string shader, const char* extension, std::string& shaderPath)
	{
		auto name = GetFileNameWithoutExtension(shader);
		
		std::string shaderOutput = BASE_SPIRV_OUTPUT + name.c_str() + extension;
		std::string command = SPIRV_COMPILER_PATH + " -V " + shaderPath + " -o " + shaderOutput;
		return command;
	}

	void CompileAllShaders()
	{
		for (auto& shader : std::filesystem::directory_iterator(BASE_SHADER_PATH))
		{
			bool isDirectory = shader.is_directory();
			const std::string& filenameStr = shader.path().filename().string();
			bool isTesselationFolder = isDirectory && filenameStr.compare("Tesselation") == 0;
			// vertex and fragment shaders
			if (!isDirectory)
			{
				auto extension= shader.path().extension().string();
				auto shaderString = shader.path().filename().string();
				std::string shaderPath =shader.path().string();
			
				std::string command="";
				if (extension == ".vert") {
					// Handle vertex shader
					const char* extension = ".spvV";
					command = CreateCommand(shaderString, extension, shaderPath);
				}
				else if (extension == ".frag") {
					const char* extension = ".spvF";
					command = CreateCommand(shaderString, extension, shaderPath);
				}
				if (command != "")
				{
					int result = std::system(command.c_str());
					if (result != 0)
					{
						Log::GetLog()->error("shader {0} is not compiled!", shaderString);
					}
				}
			}
			// tesselation shaders
			else if (isTesselationFolder)
			{
				auto tesslationFoldePath = shader;
				for (auto& shader : std::filesystem::directory_iterator(tesslationFoldePath))
				{
					std::string command = "";
					auto extension = shader.path().extension().string();
					auto shaderString = shader.path().filename().string();
					auto shaderPath = BASE_SHADER_PATH + "Tesselation/" + shaderString;
					if (extension == ".tesc") {
						const char* extension = ".spvC";
						command = CreateCommand(shaderString, extension, shaderPath);
					}
					else  if (extension == ".tese") {
				
						const char* extension = ".spvE";
						command = CreateCommand(shaderString, extension, shaderPath);
						}
				if (command != "")
				{
					int result = std::system(command.c_str());
					if (result != 0)
					{
						Log::GetLog()->error("shader {0} is not compiled!", shaderString);
					}
				}
				
				}
			
			}
		}
	}

	std::vector<std::string> CompileShader(const std::vector<std::string>& shaderFilenames)
	{
		//C:\VulkanSDK\1.3.216.0\Bin\glslc.exe shader.vert -o vertex.spv
		//C:\VulkanSDK\1.3.216.0\Bin\glslc.exe shader.frag - o fragment.spv
		//C : \VulkanSDK\1.3.216.0\Bin\glslc.exe - c shader.comp - o compute.spv
		
		auto& vertexShader = shaderFilenames[0];
		const char* extension = ".spvV";
		std::string shaderPath = BASE_SHADER_PATH + vertexShader;
		auto command = CreateCommand(vertexShader, extension, shaderPath);
		

		// Execute the command
		int result = std::system(command.c_str());
		// Check the result of the command execution
		if (result == 0)
		{
			// Command executed successfully
			// Handle the compiled SPIR-V code or other tasks
		}
		else
		{
			// Command execution failed
			// Handle the failure scenario
		}
		auto& fragmentShader = shaderFilenames[1];
		shaderPath = BASE_SHADER_PATH + fragmentShader;
		extension = ".spvF";
		command = CreateCommand(fragmentShader, extension, shaderPath);

		// Execute the command
		result = std::system(command.c_str());
		if (result == 0)
		{
			// Command executed successfully
			// Handle the compiled SPIR-V code or other tasks
		}
		else
		{
			// Command execution failed
			// Handle the failure scenario
		}
		return {};
	}


	void SetIndexForCorners(std::vector<Vertex>& plane, glm::vec2 uv )
	{

	}
	std::vector<Vertex> GenerateSphere(float radius, int rings,int sectors, std::vector<IndexType>& indices)
	{
		std::vector<Vertex> vertices;
		float const R = 1.0f / static_cast<float>(rings - 1);
		float const S = 1.0f / static_cast<float>(sectors - 1);
		{
		
			int r, s;
			float pi = 3.14159;
			for (r = 0; r < rings; ++r) {
				for (s = 0; s < sectors; ++s) {
					float const y = glm::sin(-pi * 2 + pi * r * R);
					float const x = glm::cos(2 * pi * s * S) * glm::sin(pi * r * R);
					float const z = glm::sin(2 * pi * s * S) * glm::sin(pi * r * R);

					Vertex vertex;
					vertex.x = x * radius;
					vertex.y = y * radius;
					vertex.z = z * radius;

					vertices.push_back(vertex);
				}
			}
		}

		{
			int r, s;

			for (r = 0; r < rings - 1; ++r) {
				for (s = 0; s < sectors - 1; ++s) {
					int first = r * sectors + s;
					int second = (r + 1) * sectors + s;
					int third = (r + 1) * sectors + (s + 1);
					int fourth = r * sectors + (s + 1);

					indices.push_back(first);
					indices.push_back(second);
					indices.push_back(third);

					indices.push_back(first);
					indices.push_back(third);
					indices.push_back(fourth);
				}
			}
		}
		


		return vertices;

	}
	std::vector<Vertex> GeneratePlane(float detail, std::vector<IndexType>& indices)
	{
		std::vector<Vertex> vertices;

		int numDivisions = static_cast<int>(detail);
		float stepSize = 1.0f / numDivisions;

		for (int i = 0; i <= numDivisions; ++i)
		{
			for (int j = 0; j <= numDivisions; ++j)
			{
				Vertex vertex;

				// Calculate vertex position
				vertex.x = i * stepSize - 0.5f;
				vertex.y = 0.0;
				vertex.z = j * stepSize - 0.5f;

				//// Calculate vertex normal
				//vertex.nx = 0.0f;
				//vertex.ny = 1.0f;
				//vertex.nz = 0.0f;

				// Calculate texture coordinates
				vertex.u = static_cast<float>(i) / numDivisions;
				vertex.v = static_cast<float>(j) / numDivisions;

				// Add the vertex to the vector
				vertices.push_back(vertex);
			}
		}
		// Generate indices for the plane
		for (int i = 0; i < numDivisions; ++i)
		{
			for (int j = 0; j < numDivisions; ++j)
			{
				int amountOfRowVerticies = (numDivisions + 1);
				// Calculate indices for the current quad
				unsigned int topLeft = i * amountOfRowVerticies + j;
				unsigned int topRight = topLeft + 1;
				//unsigned int bottomLeft = topLeft + amountOfRowVerticies;
				//unsigned int bottomRight = bottomLeft + 1;
				unsigned int bottomRight = topRight + amountOfRowVerticies-1;
				unsigned int bottomLeft = topLeft + amountOfRowVerticies+1;
				#define TRIANGLE 0
				#if TRIANGLE
					// Add the indices to the vector
					indices.push_back(topLeft);
					indices.push_back(bottomLeft);
					indices.push_back(topRight);
				
					indices.push_back(topRight);
					indices.push_back(bottomLeft);
					indices.push_back(bottomRight);
				#else
				indices.push_back(topLeft);
				indices.push_back(topRight);
				indices.push_back(bottomRight);
				indices.push_back(bottomLeft);
				#endif
				
			}
		}

		return vertices;
	}


	std::string InitFilePath()
	{
		std::string baseShaderPath = "";

		// Check if running within Visual Studio
		const char* visualStudioEnvVar = std::getenv("VSLANG");
		if (visualStudioEnvVar != nullptr)
		{
			// Set the base shader path relative to the project directory
			BASE_SHADER_PATH = "../Shaders/";
			BASE_RES_PATH = "../res/";

		}
		else
		{
			// Set the base shader path relative to the executable directory
			std::filesystem::path executablePath = std::filesystem::current_path();
			BASE_SHADER_PATH = executablePath.parent_path().string() + "../../../Shaders/";
			BASE_RES_PATH = executablePath.parent_path().string() + "../../../res/";
			BASE_SPIRV_OUTPUT = BASE_SHADER_PATH + "Binary/";
		}

		return baseShaderPath;
	}


	void Renderer::Init(size_t screenWidth, size_t screenHeight, std::shared_ptr<Window> window, Application* app) 
		
	{
		
		InitFilePath();
		CompileAllShaders();
		//CompileShader({ "shader.vert","shader.frag" });
		m_Window=window; 
		m_ViewportWidth = screenWidth;
		m_ViewportHeight = screenHeight;
		m_App = app;
		m_CommandPoolManager = CreateUPtr<CommandPoolManager>();
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
		

//
		
		 
	

		// create uniform buffers for each frame
	
		
		vk::DescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = 0;
		layoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
		layoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex| vk::ShaderStageFlagBits::eTessellationControl
			| vk::ShaderStageFlagBits::eTessellationEvaluation;
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
			layoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eTessellationEvaluation;
			layoutBinding.descriptorCount = 1;

			std::vector<vk::DescriptorSetLayoutBinding> layoutBindings{ layoutBinding };

			m_DescriptorSetLayoutTex = DescriptorSetLayout::Create(layoutBindings);

			layouts[0] = m_DescriptorSetLayoutTex->GetLayout();
			m_DescriptorSetTex = m_DescriptorPoolTex->AllocateDescriptorSets(1, layouts.data())[0];
		}




		auto commandPool = m_CommandPoolManager->GetFreePool();
		m_RenderCommandBuffer = CommandBuffer::CreateBuffers(commandPool, vk::CommandBufferLevel::ePrimary, 3);
		m_TransferCommandBuffer = CommandBuffer::CreateBuffers(commandPool, vk::CommandBufferLevel::ePrimary, 3);

		CreateSyncObjects();







		m_Image = Image::CreateImage(BASE_RES_PATH+"noise.jpg", m_DescriptorSetTex);
		//std::string modelPath = BASE_RES_PATH+"viking_room/viking_room.obj";
		//m_Model = Model::Load(modelPath);

		std::vector<IndexType> indices;
		auto vertices = GeneratePlane(1, indices);
		auto indexSize = SizeOfBuffer(indices.size(),indices[0]);
		{
			SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(indexSize);


			{
				BufferInputChunk inputBuffer;
				inputBuffer.size = indexSize;
				inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
				inputBuffer.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
				m_IndexBuffer = CreateUPtr<IndexBuffer>(inputBuffer, indices.size(), vk::IndexType::eUint32);

			}

			m_TransferCommandBuffer[0].BeginTransfering();
			m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_IndexBuffer.get(), (void*)indices.data(), indexSize);
			m_TransferCommandBuffer[0].EndTransfering();



		}

		auto vertexSize = SizeOfBuffer(vertices.size(), vertices[0]);
		void* vertexData = const_cast<void*>(static_cast<const void*>(vertices.data()));
		SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(vertexSize);
		{
			BufferInputChunk inputBuffer;
			inputBuffer.size = vertexSize;
			inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
			inputBuffer.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;

			m_ModelBuffer = CreateUPtr<Buffer>(inputBuffer);
		}

		m_TransferCommandBuffer[0].BeginTransfering();
		m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_ModelBuffer.get(), (void*)vertices.data(), vertexSize);
		m_TransferCommandBuffer[0].EndTransfering();


		{
			BufferInputChunk inputBuffer;
			inputBuffer.size = INSTANCE_COUNT * sizeof(InstanceData);
			// up to 256 mb
			// coherent meaning that updates made by the CPU are immediately visible to the GPU
			//vk::MemoryPropertyFlagBits::eHostCoherent
			inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;
			inputBuffer.usage = vk::BufferUsageFlagBits::eVertexBuffer;

			m_InstancedDataBuffer = CreateUPtr<Buffer>(inputBuffer);

		}
		m_InstancedPtr = m_Device->GetDevice().mapMemory(m_InstancedDataBuffer->GetMemory(), 0, INSTANCE_COUNT * sizeof(InstanceData));
		//m_InstanceData.reserve(100);



		{
			std::vector<IndexType> indices;
			auto vertices = GenerateSphere(1.,20.,20, indices);
			auto indexSize = SizeOfBuffer(indices.size(), indices[0]);
			{
				SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(indexSize);


				{
					BufferInputChunk inputBuffer;
					inputBuffer.size = indexSize;
					inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
					inputBuffer.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
					m_IndexSphereBuffer = CreateUPtr<IndexBuffer>(inputBuffer, indices.size(), vk::IndexType::eUint32);

				}

				m_TransferCommandBuffer[0].BeginTransfering();
				m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_IndexSphereBuffer.get(), (void*)indices.data(), indexSize);
				m_TransferCommandBuffer[0].EndTransfering();



			}

			auto vertexSize = SizeOfBuffer(vertices.size(), vertices[0]);
			void* vertexData = const_cast<void*>(static_cast<const void*>(vertices.data()));
			SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(vertexSize);
			{
				BufferInputChunk inputBuffer;
				inputBuffer.size = vertexSize;
				inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
				inputBuffer.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;

				m_SphereBuffer = CreateUPtr<Buffer>(inputBuffer);
			}

			m_TransferCommandBuffer[0].BeginTransfering();
			m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_SphereBuffer.get(), (void*)vertices.data(), vertexSize);
			m_TransferCommandBuffer[0].EndTransfering();
		}
		


		CreateMSAAFrame();
		CreatePipeline();
		CreateFramebuffers();
		


		auto physDev = m_Device->GetDevicePhys();
		auto dev = m_Device->GetDevice();
		auto queue = m_Device->GetGraphicsQueue();
		auto commandPoolTracy = m_CommandPoolManager->GetFreePool();
		m_TracyCommandBuffer = CommandBuffer::CreateBuffer(commandPoolTracy,vk::CommandBufferLevel::ePrimary);
		auto instance = m_Instance->GetInstance();
		PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT vkGetPhysicalDeviceCalibrateableTimeDomainsEXT = reinterpret_cast<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT"));
		PFN_vkGetCalibratedTimestampsEXT vkGetCalibratedTimestampsEXT = reinterpret_cast<PFN_vkGetCalibratedTimestampsEXT>(vkGetDeviceProcAddr(dev, "vkGetCalibratedTimestampsEXT"));

		ctx = TracyVkContextCalibrated(physDev,dev,queue, m_TracyCommandBuffer.GetCommandBuffer(),
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

		//ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//ubo.view = glm::lookAt(glm::vec3(-2.0f, 0.0f, 4.0f), glm::vec3(0.5f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		////ubo.proj = glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.0001f, 10.0f);
		ubo.view = cameraView;
		ubo.proj = cameraProj;
		//auto model = glm::mat4(1.f);
		//glm::mat4 blenderToLH = glm::mat4(1.0f);
		//blenderToLH[2][2] = -1.0f;  // Flip Z-axis
		//blenderToLH[3][2] = 1.0f;
		////model = blenderToLH * model;
		//// blender: z  is up, y is forward
		//model = glm::rotate(model,glm::radians(-90.f) , glm::vec3(1, 0, 0));
		//model = glm::rotate(model,glm::radians(90.f) , glm::vec3(0, 0, 1));
		//ubo.model = model;
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

	void Renderer::Shutdown()
	{

		m_CommandPoolManager->Release();
	}


	void Renderer::RecordCommandBuffer(uint32_t imageIndex,vk::RenderPass& renderPass,vk::Pipeline& pipeline, vk::PipelineLayout& pipelineLayout, int instances)
	{

		vk::CommandBufferBeginInfo beginInfo = {};

		auto commandBuffer = m_RenderCommandBuffer[imageIndex].GetCommandBuffer();

		commandBuffer.begin(beginInfo);
		{
			TracyVkZone(ctx, commandBuffer, "Rendering");
			m_RenderCommandBuffer[imageIndex].BeginRenderPass(&renderPass, &m_Swapchain->GetFrames()[imageIndex].framebuffer, &m_Swapchain->GetExtent());
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

			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, m_DescriptorSets[imageIndex],nullptr);
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 1, m_DescriptorSetTex, nullptr);
			
			commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
			vk::DeviceSize offsets[] = { 0 };

			{
				vk::Buffer vertexBuffers[] = { m_ModelBuffer->GetBuffer()};
				commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

			}

			{
				vk::Buffer vertexBuffers[] = { m_InstancedDataBuffer->GetBuffer()};
				commandBuffer.bindVertexBuffers(1, 1, vertexBuffers, offsets);

			}


			commandBuffer.setViewport(0, 1, &viewport);
			commandBuffer.setScissor(0, 1, &scissors);

			commandBuffer.bindIndexBuffer(m_IndexBuffer->GetBuffer(), 0, m_IndexBuffer->GetIndexType());
            auto amount = m_IndexBuffer->GetIndexAmount();
			commandBuffer.drawIndexed(static_cast<uint32_t>(amount),instances, 0, 0, 0);
			
			//commandBuffer.draw(8192, 1, 0, 0);
		
		}


		m_RenderCommandBuffer[imageIndex].EndRenderPass();
		TracyVkCollect(ctx, commandBuffer);
		commandBuffer.end();
		
	}

	size_t currentFrame = 0;
	Renderer* Renderer::Instance()
	{
		static Renderer* s_Renderer = new Renderer();
		return s_Renderer;
	}
	void Renderer::Render()
	{
		m_InstanceData.clear();
		auto cameraPos = m_App->GetCamera()->m_Position;
		//cameraPos = { 5,0,-5 };
		auto quadTree = Quadtree::Build(cameraPos);

		
		for (auto& entry : quadTree.nodes)
		{
			for (auto node : entry.second)
			{
				if (node.isDrawn)
				{	
					auto data = InstanceData{ node.worldPosition,node.tileWidth,0 };

					data.edges[0] = node.edges[0];
					data.edges[1] = node.edges[1];
					data.edges[2] = node.edges[2];
					data.edges[3] = node.edges[3];
					m_InstanceData.emplace_back(data);
				}
				
				
				
				
			}
		}
		

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
		m_TransferCommandBuffer[imageIndex].BeginTransfering();
		auto& transferCommandBuffer = m_TransferCommandBuffer[imageIndex];
		vk::BufferMemoryBarrier bufferBarrier{};
		bufferBarrier.sType = vk::StructureType::eBufferMemoryBarrier;
		bufferBarrier.srcAccessMask = vk::AccessFlagBits::eVertexAttributeRead;
		bufferBarrier.dstAccessMask = vk::AccessFlagBits::eHostWrite;
		bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarrier.buffer = m_InstancedDataBuffer->GetBuffer();
		bufferBarrier.offset = 0;
		bufferBarrier.size = VK_WHOLE_SIZE;

		transferCommandBuffer.GetCommandBuffer().pipelineBarrier(
			vk::PipelineStageFlagBits::eVertexInput,    // The pipeline stage when the barrier is inserted
			vk::PipelineStageFlagBits::eHost,           // The pipeline stage where the CPU writes to the memory
			vk::DependencyFlags{},                      // Dependency flags
			0, nullptr,                                 // Memory barriers
			1, &bufferBarrier,                          // Buffer memory barriers
			0, nullptr                                  // Image memory barriers
		);

		memcpy(m_InstancedPtr, &m_InstanceData[0],sizeof(m_InstanceData) * m_InstanceData.size());

		
		//renderCommandBuffer.GetCommandBuffer().pipelineBarrier(
		//	vk::PipelineStageFlagBits::eVertexInput,    
		//	vk::PipelineStageFlagBits::eHost,    
		//	vk::DependencyFlags{},                      
		//	0, nullptr,                                 
		//	1, &bufferBarrier,                          
		//	0, nullptr                                  
		//);
		m_TransferCommandBuffer[imageIndex].EndTransfering();
			vk::Semaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
			vk::Semaphore signalSemaphores[] = { m_RenderFinishedSemaphore };

		auto& renderCommandBuffer = m_RenderCommandBuffer[imageIndex];
		{
			ZoneScopedN("Sumbit render commands");
			renderCommandBuffer.BeginRendering();

			RecordCommandBuffer(imageIndex,m_RenderPass,m_Pipeline,m_PipelineLayout,m_InstanceData.size());
			//RecordCommandBuffer(imageIndex,  m_DebugRenderPass, m_DebugPipeline, m_DebugPipelineLayout,1);
			
			renderCommandBuffer.Submit(waitSemaphores, signalSemaphores, &m_InFlightFence);
			renderCommandBuffer.EndRendering();

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


	vk::VertexInputBindingDescription CreateBindingDescription(uint32_t binding,
		uint32_t stride,
		vk::VertexInputRate inputRate)
	{
		vk::VertexInputBindingDescription vInputBindDescription{};
		vInputBindDescription.binding = binding;
		vInputBindDescription.stride = stride;
		vInputBindDescription.inputRate = inputRate;
		return vInputBindDescription;
	}


	inline vk::VertexInputAttributeDescription VertexInputAttributeDescription(
		uint32_t binding,
		uint32_t location,
		vk::Format format,
		uint32_t offset)
	{
		vk::VertexInputAttributeDescription vInputAttribDescription{};
		vInputAttribDescription.location = location;
		vInputAttribDescription.binding = binding;
		vInputAttribDescription.format = format;
		vInputAttribDescription.offset = offset;
		return vInputAttribDescription;
	}

	float random(glm::vec2 st) {
		return glm::fract(glm::sin(glm::dot(st,
			glm::vec2(12.9898, 78.233))) *
			43758.5453123);
	}
	float lerp(float a, float b, float t)
	{
		return a + (b - a) * t;
	}
	float noise(glm::vec2 cell, glm::vec2 uv, float nextVertexOffset) {
		glm::vec2 i = cell;
		glm::vec2 f = uv;
		// i = vec2(gl_InstanceIndex,0);
		float a = random(i);
		float b = random(i + glm::vec2(nextVertexOffset, 0.0));
		float c = random(i + glm::vec2(0.0, nextVertexOffset));
		float d = random(i + glm::vec2(nextVertexOffset, nextVertexOffset));

		// smooth step function lol
		glm::vec2 u = f * f * (3.0f - 2.0f * f);
		//u = vec2(1,1);

		//return u.x;
		float interpolated;

		float interpolatedX = lerp(a, b, u.x);
		float interpolatedY = lerp(c, d, u.x);
		interpolated = lerp(interpolatedX, interpolatedY, u.y);

		return interpolated;
	}
	void Renderer::CreatePipeline()
	{
		// terrain pipeline
		GraphicsPipelineSpecification specs;

		auto swapchainFormat = m_Swapchain->GetFormat();
		auto swapChainExtent = m_Swapchain->GetExtent();
		specs.device = m_Device->GetDevice();
		
		specs.vertexFilepath = BASE_SPIRV_OUTPUT +"noise.spvV";
		specs.fragmentFilepath = BASE_SPIRV_OUTPUT +"default.spvF";
		specs.tessCFilepath = BASE_SPIRV_OUTPUT +"tessC.spvC";
		specs.tessEFilepath = BASE_SPIRV_OUTPUT +"tessE.spvE";
		specs.swapchainExtent = swapChainExtent;
		specs.swapchainImageFormat = swapchainFormat;


		std::vector<vk::VertexInputBindingDescription> bindings{ CreateBindingDescription(0,sizeof(Vertex),vk::VertexInputRate::eVertex) ,CreateBindingDescription (1,sizeof(InstanceData),vk::VertexInputRate::eInstance)};

		std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;

		attributeDescriptions =
		{
			VertexInputAttributeDescription(0,0,vk::Format::eR32G32B32Sfloat,offsetof(Vertex, Position)),
			VertexInputAttributeDescription(0,1,vk::Format::eR32G32B32A32Sfloat,offsetof(Vertex, Color)),
			VertexInputAttributeDescription(0,2,vk::Format::eR32G32B32A32Sfloat,offsetof(Vertex, noiseValue)),
			VertexInputAttributeDescription(0,3,vk::Format::eR32Sfloat,offsetof(Vertex, UV)),

			VertexInputAttributeDescription(1,4,vk::Format::eR32G32B32Sfloat,offsetof(InstanceData, pos)),
			VertexInputAttributeDescription(1,5,vk::Format::eR32G32B32A32Sfloat,offsetof(InstanceData, edges)),
			VertexInputAttributeDescription(1,6,vk::Format::eR32Sfloat,offsetof(InstanceData, scale)),
		};

		specs.bindingDescription = bindings;



		specs.attributeDescription = attributeDescriptions;
		//specs.attributeDescription = Particle::GetAttributeDescriptions();

		auto samples = RenderContext::GetDevice()->GetSamples();
		specs.samples = samples;
		m_DescriptorSetLayouts = std::vector<vk::DescriptorSetLayout>{ m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutTex->GetLayout()};

		specs.descriptorSetLayout = m_DescriptorSetLayouts;
		auto pipline = CreatePipeline(specs, vk::PrimitiveTopology::ePatchList);
		m_Pipeline = pipline.pipeline;
		m_PipelineLayout= pipline.layout;
		m_RenderPass= pipline.renderpass;

		// debug pipeline

		//{
		//	GraphicsPipelineSpecification specs;
		//
		//	auto swapchainFormat = m_Swapchain->GetFormat();
		//	auto swapChainExtent = m_Swapchain->GetExtent();
		//	specs.device = m_Device->GetDevice();
		//
		//	specs.vertexFilepath = BASE_SPIRV_OUTPUT + "debug.spvV";
		//	specs.fragmentFilepath = BASE_SPIRV_OUTPUT + "default.spvF";
		//	specs.swapchainExtent = swapChainExtent;
		//	specs.swapchainImageFormat = swapchainFormat;
		//
		//
		//	std::vector<vk::VertexInputBindingDescription> bindings{ CreateBindingDescription(0,sizeof//(Vertex),vk::VertexInputRate::eVertex) ,CreateBindingDescription(1,sizeof//(InstanceData),vk::VertexInputRate::eInstance) };
		//
		//	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
		//
		//	attributeDescriptions =
		//	{
		//		VertexInputAttributeDescription(0,0,vk::Format::eR32G32B32Sfloat,offsetof(Vertex, Position)),
		//		VertexInputAttributeDescription(0,1,vk::Format::eR32G32B32A32Sfloat,offsetof(Vertex, Color)),
		//		VertexInputAttributeDescription(0,2,vk::Format::eR32G32Sfloat,offsetof(Vertex, UV)),
		//
		//		VertexInputAttributeDescription(1,3,vk::Format::eR32G32B32Sfloat,offsetof(InstanceData, pos)),
		//		VertexInputAttributeDescription(1,4,vk::Format::eR32Sfloat,offsetof(InstanceData, scale)),
		//		VertexInputAttributeDescription(1,5,vk::Format::eR32Sint,offsetof(InstanceData, texIndex))
		//	};
		//
		//	specs.bindingDescription = bindings;
		//
		//
		//
		//	specs.attributeDescription = attributeDescriptions;
		//	//specs.attributeDescription = Particle::GetAttributeDescriptions();
		//
		//	auto samples = RenderContext::GetDevice()->GetSamples();
		//	specs.samples = samples;
		//	m_DescriptorSetLayouts = std::vector<vk::DescriptorSetLayout>{ m_DescriptorSetLayout->GetLayout//(),m_DescriptorSetLayoutTex->GetLayout() };
		//
		//	specs.descriptorSetLayout = m_DescriptorSetLayouts;
		//
		//	auto pipline = CreatePipeline(specs, vk::PrimitiveTopology::eTriangleList);
		//	m_DebugPipeline = pipline.pipeline;
		//	m_DebugPipelineLayout = pipline.layout;
		//	m_DebugRenderPass = pipline.renderpass;
		//
		//}
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

	std::unordered_map < glm::vec3 , int > indexes;
	std::vector<int> erased;
	static int id = 0;
	void GenerateChildren(std::vector<InstanceData>& tiles, glm::vec3 centerOfParentTile, float tileScale,int depth)
	{
		// assign indexes of neighbours?
		//tileScale /= 2;
		float len = glm::dot(centerOfParentTile, glm::vec3{0,0,0});
		if (indexes.find(centerOfParentTile) != indexes.end())
		{
			{
				if (std::find(erased.begin(), erased.end(), indexes.at(centerOfParentTile)) == erased.end())
				{
					erased.push_back(indexes[centerOfParentTile]);
				}
			}
		glm::vec3 leftTop;
		leftTop.x = centerOfParentTile.x + tileScale/2 ;
		leftTop.y = 0.f;
		leftTop.z = centerOfParentTile.z + tileScale/2;
		glm::vec3 rightTop;

		rightTop.x = centerOfParentTile.x -tileScale/2;
		rightTop.y = 0.f;
		rightTop.z = centerOfParentTile.z + tileScale/2 ;

		glm::vec3 leftBottom;

		leftBottom.x = centerOfParentTile.x + tileScale/2;
		leftBottom.y = 0.f;
		leftBottom.z = centerOfParentTile.z - tileScale/2;


		glm::vec3 rightBottom;


		rightBottom.x = centerOfParentTile.x - tileScale/2 ;
		rightBottom.y = 0.f;
		rightBottom.z = centerOfParentTile.z - tileScale/2 ;
		tiles.emplace_back(leftTop, tileScale, ++id);
		indexes[leftTop] = id;
		tiles.emplace_back(rightTop, tileScale,  ++id);
		indexes[rightTop] = id;
		tiles.emplace_back(leftBottom, tileScale,  ++id);
		indexes[leftBottom] = id;
		tiles.emplace_back(rightBottom, tileScale,  ++id);
		indexes[rightBottom] = id;
		}
		
		
	}
	//tile scale is width and height of tile
	void GenerateLeftTopChildren(std::vector<InstanceData>& tiles,glm::vec3 centerOfParentTile,float tileScale)
	{
		glm::vec3 leftTop;
		leftTop.x = centerOfParentTile.x +tileScale*2 - tileScale /2;
		leftTop.y = 0.f;
		leftTop.z = centerOfParentTile.z + tileScale * 2  - tileScale / 2;
		glm::vec3 rightTop;
		
		rightTop.x = centerOfParentTile.x - tileScale / 2;
		rightTop.y = 0.f;
		rightTop.z = centerOfParentTile.z + tileScale + tileScale / 2;
		
		glm::vec3 leftBottom;
		
		leftBottom.x = centerOfParentTile.x + tileScale * 2 - tileScale / 2;
		leftBottom.y = 0.f;
		leftBottom.z = centerOfParentTile.z+tileScale / 2;
		
		
		glm::vec3 rightBottom;
		
		
		rightBottom.x = centerOfParentTile.x + tileScale / 2;
		rightBottom.y = 0.f;
		rightBottom.z = centerOfParentTile.z+tileScale / 2;

		tiles.emplace_back(leftTop, tileScale, 0);
		tiles.emplace_back(rightTop, tileScale, 0);
		tiles.emplace_back(leftBottom, tileScale, 0);
		tiles.emplace_back(rightBottom, tileScale, 0);
	}
	void GenerateRightTopChildren(std::vector<InstanceData>& tiles, glm::vec3 centerOfParentTile, float tileScale)
	{
		glm::vec3 leftTop;
		leftTop.x = centerOfParentTile.x - tileScale  + tileScale / 2;
		leftTop.y = 0.f;
		leftTop.z = centerOfParentTile.z + tileScale * 2 - tileScale / 2;
		glm::vec3 rightTop;

		rightTop.x = centerOfParentTile.x - tileScale *2 + tileScale / 2;
		rightTop.y = 0.f;
		rightTop.z = centerOfParentTile.z + tileScale * 2 - tileScale / 2;

		glm::vec3 leftBottom;

		leftBottom.x = centerOfParentTile.x - tileScale + tileScale / 2;
		leftBottom.y = 0.f;
		leftBottom.z = centerOfParentTile.z + tileScale / 2;


		glm::vec3 rightBottom;


		rightBottom.x = centerOfParentTile.x - tileScale * 2 + tileScale / 2;
		rightBottom.y = 0.f;
		rightBottom.z = centerOfParentTile.z + tileScale / 2;

		tiles.emplace_back(leftTop, tileScale, 0);
		tiles.emplace_back(rightTop, tileScale, 0);
		tiles.emplace_back(leftBottom, tileScale, 0);
		tiles.emplace_back(rightBottom, tileScale, 0);
	}

	void GenerateRightBottomChildren(std::vector<InstanceData>& tiles, glm::vec3 centerOfParentTile, float tileScale)
	{
		glm::vec3 leftTop;
		leftTop.x = centerOfParentTile.x - tileScale + tileScale / 2;
		leftTop.y = 0.f;
		leftTop.z = centerOfParentTile.z -  tileScale / 2;
		glm::vec3 rightTop;

		rightTop.x = centerOfParentTile.x - tileScale * 2 + tileScale / 2;
		rightTop.y = 0.f;
		rightTop.z = centerOfParentTile.z - tileScale / 2;

		glm::vec3 leftBottom;

		leftBottom.x = centerOfParentTile.x - tileScale/2;
		leftBottom.y = 0.f;
		leftBottom.z = centerOfParentTile.z - tileScale * 2 + tileScale / 2;


		glm::vec3 rightBottom;


		rightBottom.x = centerOfParentTile.x - tileScale * 2 + tileScale / 2;
		rightBottom.y = 0.f;
		rightBottom.z = centerOfParentTile.z - tileScale * 2 + tileScale / 2;

		tiles.emplace_back(leftTop, tileScale, 0);
		tiles.emplace_back(rightTop, tileScale, 0);
		tiles.emplace_back(leftBottom, tileScale, 0);
		tiles.emplace_back(rightBottom, tileScale, 0);
	}
	void GenerateLeftBottomChildren(std::vector<InstanceData>& tiles, glm::vec3 centerOfParentTile, float tileScale)
	{
		glm::vec3 leftTop;
		leftTop.x = centerOfParentTile.x + tileScale * 2 - tileScale / 2;
		leftTop.y = 0.f;
		leftTop.z = centerOfParentTile.z - tileScale / 2;
		glm::vec3 rightTop;

		rightTop.x = centerOfParentTile.x + tileScale / 2;
		rightTop.y = 0.f;
		rightTop.z = centerOfParentTile.z - tileScale / 2;

		glm::vec3 leftBottom;

		leftBottom.x = centerOfParentTile.x + tileScale * 2 - tileScale / 2;
		leftBottom.y = 0.f;
		leftBottom.z = centerOfParentTile.z - tileScale * 2 + tileScale / 2;


		glm::vec3 rightBottom;


		rightBottom.x = centerOfParentTile.x + tileScale / 2;
		rightBottom.y = 0.f;
		rightBottom.z = centerOfParentTile.z - tileScale * 2 + tileScale / 2;

		tiles.emplace_back(leftTop, tileScale, 0);
		tiles.emplace_back(rightTop, tileScale, 0);
		tiles.emplace_back(leftBottom, tileScale, 0);
		tiles.emplace_back(rightBottom, tileScale, 0);
	}

	
	float c = 5.f;
	
	void Renderer::GenerateTerrain(glm::vec3 tilePos,float depth,float tileWidthOfTileToDivide, int parentIndex)
	{
		glm::vec3 posPlayer = m_App->GetCamera()->m_Position;
		auto distance = posPlayer - tilePos;
		bool isDecreaseRes = (tileWidthOfTileToDivide*2 / glm::length(distance) )< c;
#if ZEROPOS
		posPlayer = glm::vec3{ 0,0,0 };
#endif // DEBUG

		
		//if (depth >= levelOfDetail && isDecreaseRes)
		//{
		//	return;
		//}
		auto dirToPlayer = glm::normalize(distance);
		bool isTop = glm::dot(dirToPlayer, glm::vec3{ 0,0,1 }) > 0;
		float side = glm::dot(dirToPlayer, glm::vec3{ 1,0,0 });



		// element after parent index elements of begin
		//m_InstanceData.erase(m_InstanceData.begin() + parentIndex);
		
		

		GenerateChildren(m_InstanceData, tilePos, tileWidthOfTileToDivide,depth);
		const glm::vec3 rightOffset{ -tileWidthOfTileToDivide * 2 ,0,0 };
		const glm::vec3 upOffset{  0,0,tileWidthOfTileToDivide * 2 };
		const glm::vec3 bottomOffset{ 0,0,-tileWidthOfTileToDivide * 2 };
		const glm::vec3 leftOffset{ tileWidthOfTileToDivide * 2,0,0};
		const glm::vec3 leftTopOffset{ tileWidthOfTileToDivide * 2,0,tileWidthOfTileToDivide * 2 };
		const glm::vec3 rightTopOffset{ -tileWidthOfTileToDivide * 2,0,tileWidthOfTileToDivide * 2 };
		const glm::vec3 rightBottomOffset{ -tileWidthOfTileToDivide * 2,0,-tileWidthOfTileToDivide * 2 };
		const glm::vec3 leftBottomOffset{ tileWidthOfTileToDivide * 2,0,-tileWidthOfTileToDivide * 2 };

		
		

		GenerateChildren(m_InstanceData, tilePos+ rightOffset, tileWidthOfTileToDivide,depth);
		GenerateChildren(m_InstanceData, tilePos+ upOffset, tileWidthOfTileToDivide,depth);
		GenerateChildren(m_InstanceData, tilePos +bottomOffset, tileWidthOfTileToDivide,depth);
		GenerateChildren(m_InstanceData, tilePos+ leftOffset, tileWidthOfTileToDivide,depth);
		GenerateChildren(m_InstanceData, tilePos+ leftTopOffset, tileWidthOfTileToDivide,depth);
		GenerateChildren(m_InstanceData, tilePos+ rightTopOffset, tileWidthOfTileToDivide,depth);
		GenerateChildren(m_InstanceData, tilePos+ rightBottomOffset, tileWidthOfTileToDivide,depth);
		//GenerateChildren(m_InstanceData, tilePos+ leftBottomOffset, tileWidthOfTileToDivide,depth);


		//auto iterator = indexes.find(glm::length(centerOfParentTile));
		//if (iterator != indexes.end()) {
		//
		//	tiles.erase(tiles.begin() + indexes[glm::length(centerOfParentTile)]);
		//}

		//if (side > 0 && isTop)
		{
			// left top
			//GenerateLeftTopChildren(m_InstanceData, currentTilePos, tileWidth );
		}
		//else if (side < 0 && isTop)
		{
			// right top
			//GenerateRightTopChildren(m_InstanceData, currentTilePos, tileWidth  );
		}
		//else if (side > 0 && !isTop)
		{
			// left bottom
			//currentTilePos += glm::vec3{ tileWidth  ,0,-tileWidth };
			//GenerateLeftBottomChildren(m_InstanceData, currentTilePos, tileWidth);
		}
		//else if (side < 0 && !isTop)
		{
			// right bottom
			//GenerateRightBottomChildren(m_InstanceData, currentTilePos, tileWidth );
		}


 		auto tileLeftTop = tilePos + glm::vec3{ tileWidthOfTileToDivide /2  ,0,tileWidthOfTileToDivide /2 };
		auto tileRightTop = tilePos + glm::vec3{ -tileWidthOfTileToDivide /2  ,0,tileWidthOfTileToDivide /2};
		auto tileLeftBottom = tilePos + glm::vec3{ tileWidthOfTileToDivide /2 ,0,-tileWidthOfTileToDivide /2 };
		auto tileRightBottom = tilePos + glm::vec3{ -tileWidthOfTileToDivide /2  ,0,-tileWidthOfTileToDivide /2 };

		// Calculate distances
		float distLeftTop = glm::distance(posPlayer, tileLeftTop);
		float distRightTop = glm::distance(posPlayer, tileRightTop);
		float distLeftBottom = glm::distance(posPlayer, tileLeftBottom);
		float distRightBottom = glm::distance(posPlayer, tileRightBottom);

		// Find the position closest to posPlayer and get new parent index from generated tiles
		glm::vec3 closestTilePos;
		if (distLeftTop <= distRightTop && distLeftTop <= distLeftBottom && distLeftTop <= distRightBottom)
		{
			closestTilePos = tileLeftTop;
			parentIndex = std::distance(m_InstanceData.begin(), m_InstanceData.end() - 4);
		}
		else if (distRightTop <= distLeftTop && distRightTop <= distLeftBottom && distRightTop <= distRightBottom)
		{
			closestTilePos = tileRightTop;
			parentIndex = std::distance(m_InstanceData.begin(), m_InstanceData.end() - 3);
		}
		else if (distLeftBottom <= distLeftTop && distLeftBottom <= distRightTop && distLeftBottom <= distRightBottom)
		{
			closestTilePos = tileLeftBottom;
			parentIndex = std::distance(m_InstanceData.begin(), m_InstanceData.end() - 2);
		}
		else
		{
			closestTilePos = tileRightBottom;
			parentIndex = std::distance(m_InstanceData.begin(), m_InstanceData.end() - 1);
		}


		//GenerateTerrain(tilePos + glm::vec3(0, 0, tileWidthOfTileToDivide * 2),depth, tileWidthOfTileToDivide,parentIndex);
		tileWidthOfTileToDivide /=  2;
		
		GenerateTerrain(closestTilePos,++depth, tileWidthOfTileToDivide,parentIndex);
	}
	void Renderer::GenerateTerrain()
	{
//		glm::vec3 posPlayer = m_App->GetCamera()->m_Position;
//		
//#if ZEROPOS
//
//		posPlayer = glm::vec3{ 0,0,0 };
//#endif // ZEROPOS
//
//		glm::vec3 currentTilePos;
//		float depth = 0;
//		float shortestPath =  1000;
//		float longestPath =  0;
//		// generate children
//
//		float currentTileWidth = groundSize / static_cast<float>(widthGround);
//		float currentTileHeight = groundSize / static_cast<float>(heightGround); ;
//		glm::vec3 centerOffset = { currentTileWidth /2,0.,currentTileHeight /2 };
//		glm::vec3 currentTilePosBiggest = {0,0,0};
//		bool isBiggestFound = false;
//		uint32_t index = 0;
//		for (int i = -(widthGround/2 + widthGround%2); i < (widthGround/2); i++)
//		{
//			for (int j = -(heightGround / 2 + heightGround % 2); j < (heightGround / 2 ); j++)
//			{
//				//for generate children by direction
//				//glm::vec3 position = glm::vec3(i * newTileWidth, 0, j * newTileHeight) + centerOffset;
//				// just generate children
//				glm::vec3 position = glm::vec3(i * currentTileWidth, 0, j * currentTileHeight);
//				if (shortestPath > glm::length(posPlayer - position))
//				{
//					if (!isBiggestFound)
//					{
//						isBiggestFound = true;
//						currentTilePosBiggest = position;
//					}
//					currentTilePos = position;
//					index = m_InstanceData.size();
//					shortestPath = glm::length(posPlayer - position);
//
//				}
//				m_InstanceData.emplace_back(position, currentTileWidth,++id);
//				indexes[position] = id;
//			}
//		
//		}
//		//glm::vec3 position = glm::vec3(0, 0, 0);
//		//currentTileWidth = position;
//		//m_InstanceData.emplace_back(position, currentTileWidth, 1);
//		float newTileWidth = currentTileWidth / 2;
//		GenerateTerrain(currentTilePos,2, newTileWidth , index);
//		
//
//
//		for (auto erase : erased)
//		{
//			m_InstanceData.erase(
//				std::remove_if(
//					m_InstanceData.begin(),
//					m_InstanceData.end(),
//					[erase](const InstanceData& instance) {
//						return std::find(erased.begin(), erased.end(), instance.texIndex) != erased.end();
//					}
//				),
//				m_InstanceData.end()
//						);
//		}
//
//		// generate noise for each vertex
//		for (auto instance : m_InstanceData)
//		{
//
//		}
//
//
//		erased.clear();
//		indexes.clear();
//		id = 0;
		
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

	GraphicsPipeline Renderer::CreatePipeline(GraphicsPipelineSpecification& spec, vk::PrimitiveTopology topology)
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

		if (spec.tessCFilepath != "")
		{	
			vk::PipelineTessellationStateCreateInfo tesselationState{};
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
		rasterizer.polygonMode = m_PolygoneMode;
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
		if (tessControl)
		{
			spec.device.destroyShaderModule(tessControl);
			spec.device.destroyShaderModule(tessEvaulation);
		}
	
		Log::GetLog()->info("Pipeline is Created!");
		return output;


	}
	void Renderer::DestroySwapchain()
	{
		

	}

	void Renderer::SubmitInstanceData(const InstanceData& instance)
	{
		m_InstanceData.emplace_back(instance);
	}

	Renderer::~Renderer()
	{



		auto device = m_Device->GetDevice();
		device.waitIdle();
		

		Shutdown();

		TracyVkDestroy(ctx)
		if (m_ModelBuffer != nullptr)
		{
			m_ModelBuffer.reset();
			m_IndexBuffer.reset();

		}
		m_Image.reset();
		
		m_Device->GetDevice().freeMemory(m_MsaaImageMemory);
		m_Device->GetDevice().destroyImage(m_MsaaImage);
		m_Device->GetDevice().destroyImageView(m_MsaaImageView);

		//m_GraphicsCommandBuffer.GetCommandBuffer().reset();
		//m_GraphicsCommandBuffer.Free();

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

		m_Device->GetDevice().destroyPipeline(m_DebugPipeline);
		m_Device->GetDevice().destroyPipelineLayout(m_DebugPipelineLayout);


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
