
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
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "Pipeline.h"
#include <random>
#include "Initializers.h"
#include "input.h"

namespace Voidstar
{

	

	TracyVkCtx ctx;
	vk::ShaderModule CreateModule(std::string filename, vk::Device device);
	const uint32_t MAX_POINTS = 20;
	std::string BASE_SHADER_PATH = "../Shaders/";
	std::string BASE_RES_PATH = "res";
	//const std::string SPIRV_COMPILER_PATH = "C:/VulkanSDK/1.3.216.0/Bin/glslc.exe";
	const std::string SPIRV_COMPILER_PATH = "C:/VulkanSDK/1.3.216.0/Bin/glslangvalidator.exe";
	std::string BASE_SPIRV_OUTPUT = BASE_SHADER_PATH+"Binary/";
	#define INSTANCE_COUNT 4096
	#define ZEROPOS 1
	#define	IMGUI_ENABLED 1
	size_t currentFrame = 0;
	static float exeTime = 24;
	// noise
	const float noiseTextureWidth = 256.f;
	const float noiseTextureHeight = 256.f;
	// Data
	static VkAllocationCallbacks* g_Allocator = nullptr;
	static VkInstance               g_Instance = VK_NULL_HANDLE;
	static VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
	static VkDevice                 g_Device = VK_NULL_HANDLE;
	static uint32_t                 g_QueueFamily = (uint32_t)-1;
	static VkQueue                  g_Queue = VK_NULL_HANDLE;
	static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
	static VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
	static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;

	static ImGui_ImplVulkanH_Window g_MainWindowData;
	static int                      g_MinImageCount = 2;
	static bool                     g_SwapChainRebuild = false;


	std::vector<Vertex> GetCube()
	{
		const std::vector<uint32_t> indices =
		{
			0, 1, 3, 3, 1, 2,
			1, 5, 2, 2, 5, 6,
			5, 4, 6, 6, 4, 7,
			4, 0, 7, 7, 0, 3,
			3, 2, 7, 7, 2, 6,
			4, 5, 0, 0, 5, 1
		};

		std::vector<Vertex> CubeVerticies;
		CubeVerticies.resize(8);
		//vertices[0].Position = { -1, -1, -1 };
		CubeVerticies[0].Position[0] = -1;
		CubeVerticies[0].Position[1] = -1;
		CubeVerticies[0].Position[2] = -1;


		//vertices[1].Position = { 1, -1, -1 };
		CubeVerticies[1].Position[0] = 1;
		CubeVerticies[1].Position[1] = -1;
		CubeVerticies[1].Position[2] = -1;
		//vertices[2].Position = { 1, 1, -1};
		CubeVerticies[2].Position[0] = 1;
		CubeVerticies[2].Position[1] = 1;
		CubeVerticies[2].Position[2] = -1;
		//vertices[3].Position = { -1, 1, -1 };
		CubeVerticies[3].Position[0] = -1;
		CubeVerticies[3].Position[1] = 1;
		CubeVerticies[3].Position[2] = -1;
		//vertices[4].Position = { -1, -1, 1 };
		CubeVerticies[4].Position[0] = -1;
		CubeVerticies[4].Position[1] = -1;
		CubeVerticies[4].Position[2] = 1;
		//vertices[5].Position = { 1, -1, 1};
		CubeVerticies[5].Position[0] = 1;
		CubeVerticies[5].Position[1] = -1;
		CubeVerticies[5].Position[2] = 1;
		//vertices[6].Position = { 1, 1, 1};
		CubeVerticies[6].Position[0] = 1;
		CubeVerticies[6].Position[1] = 1;
		CubeVerticies[6].Position[2] = 1;
		//vertices[7].Position = { -1, 1, 1 };
		CubeVerticies[7].Position[0] = -1;
		CubeVerticies[7].Position[1] = 1;
		CubeVerticies[7].Position[2] = 1;

		CubeVerticies[0].UV[0] = 0.0f; // U coordinate
		CubeVerticies[0].UV[1] = 0.0f; // V coordinate

		CubeVerticies[1].UV[0] = 1.0f;
		CubeVerticies[1].UV[1] = 0.0f;

		CubeVerticies[2].UV[0] = 1.0f;
		CubeVerticies[2].UV[1] = 1.0f;

		CubeVerticies[3].UV[0] = 0.0f;
		CubeVerticies[3].UV[1] = 1.0f;

		CubeVerticies[4].UV[0] = 0.0f;
		CubeVerticies[4].UV[1] = 0.0f;

		CubeVerticies[5].UV[0] = 1.0f;
		CubeVerticies[5].UV[1] = 0.0f;

		CubeVerticies[6].UV[0] = 1.0f;
		CubeVerticies[6].UV[1] = 1.0f;

		CubeVerticies[7].UV[0] = 0.0f;
		CubeVerticies[7].UV[1] = 1.0f;
		return CubeVerticies;
	}


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
			bool isComputeFolder = isDirectory && filenameStr.compare("Compute") == 0;
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
			else if (isComputeFolder)
			{
				auto computeFolderPath = shader;
				for (auto& shader : std::filesystem::directory_iterator(computeFolderPath))
				{
					std::string command = "";
					auto extension = shader.path().extension().string();
					auto shaderString = shader.path().filename().string();
					auto shaderPath = BASE_SHADER_PATH + "Compute/" + shaderString;
					if (extension == ".comp") {
						const char* extension = ".spvCmp";
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
		std::vector<Vertex> vertices ={};

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
				unsigned int bottomLeft = topLeft + amountOfRowVerticies;
				unsigned int bottomRight = bottomLeft + 1;
				//unsigned int bottomRight = topRight + amountOfRowVerticies-1;
				//unsigned int bottomLeft = topLeft + amountOfRowVerticies+1;
				#define TRIANGLE 1
				#if TRIANGLE
					// Add the indices to the vector
					indices.push_back(topLeft);
					indices.push_back(bottomLeft);
					indices.push_back(topRight);
				
					indices.push_back(topRight);
					indices.push_back(bottomLeft);
					indices.push_back(bottomRight);
				#else
				// patch
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


	

	void Renderer::CreateComputePipeline()
	{

	

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
	
		
		auto framesAmount = m_Swapchain->m_SwapchainFrames.size();

		vk::DescriptorSetLayoutBinding layoutBinding = DescriptorBindingDescription(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eTessellationControl
			| vk::ShaderStageFlagBits::eTessellationEvaluation | vk::ShaderStageFlagBits::eFragment,1);
		

		
		std::vector<vk::DescriptorSetLayoutBinding> layoutBindings{ layoutBinding};
		m_DescriptorSetLayout = DescriptorSetLayout::Create(layoutBindings);

	
		


		auto bufferSize = sizeof(UniformBufferObject);
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
			poolSize.descriptorCount = static_cast<uint32_t>(framesAmount);
			vk::DescriptorPoolSize poolSize1;
			poolSize1.type = vk::DescriptorType::eCombinedImageSampler;
			poolSize1.descriptorCount = static_cast<uint32_t>(framesAmount);

			std::vector<vk::DescriptorPoolSize> poolSizes{ poolSize,poolSize1 };
			
			m_DescriptorPool = DescriptorPool::Create(poolSizes, framesAmount);
		}
		

		std::vector<vk::DescriptorSetLayout> layouts(framesAmount, m_DescriptorSetLayout->GetLayout());

		
		m_DescriptorSets =  m_DescriptorPool->AllocateDescriptorSets(framesAmount, layouts.data());
			
		for (size_t i = 0; i < framesAmount; i++)
		{
			m_Device->UpdateDescriptorSet(m_DescriptorSets[i], 0, 1, *m_UniformBuffers[i], vk::DescriptorType::eUniformBuffer);
		}

		/*m_NoiseImage = Image::CreateImage(BASE_RES_PATH + "/dos_2_noise.png");
		for (int i = 0; i < framesAmount; i++)
		{
		
			
			
			vk::DescriptorImageInfo imageInfo = DescriptorImageInfo(vk::ImageLayout::eShaderReadOnlyOptimal, m_NoiseImage->m_ImageView, m_NoiseImage->m_Sampler);
		
			m_Device->UpdateDescriptorSet(m_DescriptorSets[i], 1, 1, imageInfo, vk::DescriptorType::eCombinedImageSampler);

		}*/
		

		




		m_FrameCommandPool = m_CommandPoolManager->GetFreePool();
		m_RenderCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, 3);
		m_TransferCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, 3);
		m_ComputeCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, 3);

		CreateSyncObjects();








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
			m_TransferCommandBuffer[0].SubmitSingle();



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
		m_TransferCommandBuffer[0].SubmitSingle();


			


			
		

		CreateMSAAFrame();


		{

			vk::DescriptorPoolSize poolSize;
			poolSize.type = vk::DescriptorType::eCombinedImageSampler;
			poolSize.descriptorCount = 6;

			std::vector<vk::DescriptorPoolSize> poolSizes{ poolSize };

			m_DescriptorPoolTex = DescriptorPool::Create(poolSizes, 1);






			




			std::vector<vk::DescriptorSetLayoutBinding> layoutBindings{ 
				DescriptorBindingDescription(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1) ,
				DescriptorBindingDescription(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1) };

			m_DescriptorSetLayoutTex = DescriptorSetLayout::Create(layoutBindings);
			std::vector<vk::DescriptorSetLayout> layouts;
			layouts.resize(1);

			layouts[0] = m_DescriptorSetLayoutTex->GetLayout();
			m_DescriptorSetTex = m_DescriptorPoolTex->AllocateDescriptorSets(1, layouts.data())[0];
		}


		std::vector<vk::DescriptorPoolSize> poolSizes;
		{
			vk::DescriptorPoolSize poolSize;
			poolSize.type = vk::DescriptorType::eStorageImage;
			poolSize.descriptorCount = 1;
			poolSizes.emplace_back(poolSize);
		}


		{
			vk::DescriptorPoolSize poolSize;
			poolSize.type = vk::DescriptorType::eCombinedImageSampler;
			poolSize.descriptorCount = 1;
			poolSizes.emplace_back(poolSize);
		}

		{
			vk::DescriptorPoolSize poolSize;
			poolSize.type = vk::DescriptorType::eUniformBuffer;
			poolSize.descriptorCount = 1;
			poolSizes.emplace_back(poolSize);
			m_DescriptorPoolSelected = DescriptorPool::Create(poolSizes, 1);
		}

		layouts.resize(1);


		{
			vk::DescriptorSetLayoutBinding layoutBinding1;
			layoutBinding1.binding = 0;
			layoutBinding1.descriptorType = vk::DescriptorType::eStorageImage;
			layoutBinding1.stageFlags = vk::ShaderStageFlagBits::eFragment |
				vk::ShaderStageFlagBits::eCompute;
			layoutBinding1.descriptorCount = 1;

			vk::DescriptorSetLayoutBinding layoutBinding2;
			layoutBinding2.binding = 1;
			layoutBinding2.descriptorType = vk::DescriptorType::eStorageBuffer;
			layoutBinding2.stageFlags = vk::ShaderStageFlagBits::eCompute;
			layoutBinding2.descriptorCount = 1;


			vk::DescriptorSetLayoutBinding layoutBinding3;
			layoutBinding3.binding = 2;
			layoutBinding3.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			layoutBinding3.stageFlags = vk::ShaderStageFlagBits::eCompute;
			layoutBinding3.descriptorCount = 1;


			std::vector<vk::DescriptorSetLayoutBinding> layoutBindings{ layoutBinding1, layoutBinding2,layoutBinding3 };

			m_DescriptorSetLayoutSelected = DescriptorSetLayout::Create(layoutBindings);

		}
		
		{
			BufferInputChunk info;
			info.size = sizeof(glm::vec2) * MAX_POINTS;
			info.usage = vk::BufferUsageFlagBits::eStorageBuffer |
				vk::BufferUsageFlagBits::eTransferDst;
			info.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | 
				vk::MemoryPropertyFlagBits::eHostCoherent;
			m_ShaderStorageBuffer = CreateUPtr<Buffer>(info);
		}
		
		



		m_ImageSelected = Image::CreateEmptyImage(noiseTextureWidth, noiseTextureHeight, vk::Format::eR8G8B8A8Snorm);
		m_Image = Image::CreateImage(BASE_RES_PATH + "dos_2_noise.png");

		vk::DescriptorImageInfo imageDescriptor;
		imageDescriptor.imageLayout = vk::ImageLayout::eGeneral;
		imageDescriptor.imageView = m_ImageSelected->m_ImageView;
		imageDescriptor.sampler = m_ImageSelected->m_Sampler;

		m_DescriptorSetSelected = m_DescriptorPoolSelected->AllocateDescriptorSets(1, { &m_DescriptorSetLayoutSelected ->GetLayout()})[0];
		


		m_ClickPoints.resize(MAX_POINTS, glm::vec2(-1,-1));
		//m_ClickPoints[0] = glm::vec2{ 0.5,0.5 };
		//m_ClickPoints[1] = glm::vec2{ 0.5,0.2 };
		UpdateBuffer();
		
		vk::DescriptorImageInfo imageDescriptor1;
		imageDescriptor1.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imageDescriptor1.imageView = m_Image->m_ImageView;
		imageDescriptor1.sampler = m_Image->m_Sampler;
		m_Device->UpdateDescriptorSet(m_DescriptorSetSelected, 0,1, imageDescriptor, vk::DescriptorType::eStorageImage);
		m_Device->UpdateDescriptorSet(m_DescriptorSetSelected, 2, 1, *m_Image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eCombinedImageSampler);
		m_Device->UpdateDescriptorSet(m_DescriptorSetTex, 0, 1, *m_ImageSelected, vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eCombinedImageSampler);
		m_Device->UpdateDescriptorSet(m_DescriptorSetTex, 1, 1, *m_Image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eCombinedImageSampler);		
		layouts = { m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutSelected->GetLayout() };
		m_ComputePipeline = Pipeline::CreateComputePipeline(BASE_SPIRV_OUTPUT + "SelectedTex.spvCmp", layouts);
		UpdateTexture();


















		CreatePipeline();
		CreateFramebuffers();


		auto physDev = m_Device->GetDevicePhys();
		auto dev = m_Device->GetDevice();
		auto queue = m_Device->GetGraphicsQueue();
		m_TracyCommandPool = m_CommandPoolManager->GetFreePool();
		m_TracyCommandBuffer = CommandBuffer::CreateBuffer(m_TracyCommandPool,vk::CommandBufferLevel::ePrimary);
		auto instance = m_Instance->GetInstance();
		PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT vkGetPhysicalDeviceCalibrateableTimeDomainsEXT = reinterpret_cast<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT"));
		PFN_vkGetCalibratedTimestampsEXT vkGetCalibratedTimestampsEXT = reinterpret_cast<PFN_vkGetCalibratedTimestampsEXT>(vkGetDeviceProcAddr(dev, "vkGetCalibratedTimestampsEXT"));

		ctx = TracyVkContextCalibrated(physDev,dev,queue, m_TracyCommandBuffer.GetCommandBuffer(),
			vkGetPhysicalDeviceCalibrateableTimeDomainsEXT, vkGetCalibratedTimestampsEXT);


#if IMGUI_ENABLED
		InitImGui();
#endif	

	}

	void Renderer::SetupVulkanWindow(ImGui_ImplVulkanH_Window* g_wd, VkSurfaceKHR surface, int width, int height)
	{
		g_wd->Surface = surface;

		// Check for WSI support
		VkBool32 res;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_Device->GetDevicePhys(), m_Device->GetGraphicsIndex(), g_wd->Surface, &res);
		if (res != VK_TRUE)
		{
			fprintf(stderr, "Error no WSI support on physical device 0\n");
			exit(-1);
		}

		// Select Surface Format
		const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
		const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		g_wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(m_Device->GetDevicePhys(), g_wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

		// Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
		VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
		VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
		g_wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(m_Device->GetDevicePhys(), g_wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
		//printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

		// Create SwapChain, RenderPass, Framebuffer, etc.
		IM_ASSERT(g_MinImageCount >= 2);
		ImGui_ImplVulkanH_CreateOrResizeWindow(m_Instance->GetInstance(), m_Device->GetDevicePhys(), m_Device->GetDevice(), g_wd, m_Device->GetGraphicsIndex(), NULL, width, height, g_MinImageCount);
	}
	void Renderer::InitImGui()
	{
		// Create Descriptor Pool
		{
			VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};
			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
			pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;
			vkCreateDescriptorPool(m_Device->GetDevice(), &pool_info, VK_NULL_HANDLE, &imguiData.g_DescriptorPool);
		}

		g_MinImageCount = m_Swapchain->m_SwapchainFrames.size();
		g_Device = m_Device->GetDevice();
		g_QueueFamily = m_Device->GetGraphicsIndex();
		g_Queue = m_Device->GetGraphicsQueue();
		imguiData.g_CommandPool = m_CommandPoolManager->GetFreePool();
		imguiData.g_CommandBuffers = CommandBuffer::CreateBuffers(imguiData.g_CommandPool, vk::CommandBufferLevel::ePrimary, m_Swapchain->m_SwapchainFrames.size());
		// Create the Render Pass
		{
			VkAttachmentDescription attachment = {};
			attachment.format = (VkFormat)m_Swapchain->m_SwapchainFormat;
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			VkAttachmentReference color_attachment = {};
			color_attachment.attachment = 0;
			color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &color_attachment;
			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			VkRenderPassCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			info.attachmentCount = 1;
			info.pAttachments = &attachment;
			info.subpassCount = 1;
			info.pSubpasses = &subpass;
			info.dependencyCount = 1;
			info.pDependencies = &dependency;
			auto err = vkCreateRenderPass(m_Device->GetDevice(), &info, nullptr, &imguiData.g_RenderPass);

		}
		g_MainWindowData.RenderPass = imguiData.g_RenderPass;
		
		
			
		auto g_wd= &g_MainWindowData;
		g_wd->Width = m_ViewportWidth;
		g_wd->Height = m_ViewportHeight;
		g_wd->Surface = m_Surface;
		g_wd->ImageCount= m_Swapchain->m_SwapchainFrames.size();
		g_wd->PresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		g_wd->SurfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
		g_wd->Swapchain = m_Swapchain->m_Swapchain;

		{
			VkImageView attachment[1];
			VkFramebufferCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			info.renderPass = g_wd->RenderPass;
			info.attachmentCount = 1;
			info.pAttachments = attachment;
			info.width = g_wd->Width;
			info.height = g_wd->Height;
			info.layers = 1;
			g_wd->Frames = new ImGui_ImplVulkanH_Frame[g_wd->ImageCount];
			for (uint32_t i = 0; i < g_wd->ImageCount; i++)
			{
				g_wd->Frames[i].Backbuffer = m_Swapchain->m_SwapchainFrames[i].image;
				g_wd->Frames[i].BackbufferView = m_Swapchain->m_SwapchainFrames[i].imageView;
				g_wd->Frames[i].CommandBuffer = imguiData.g_CommandBuffers[i].GetCommandBuffer();
				ImGui_ImplVulkanH_Frame* fd = &g_wd->Frames[i];
				attachment[0] = fd->BackbufferView;
				auto err = vkCreateFramebuffer(m_Device->GetDevice(), &info, nullptr, &fd->Framebuffer);
			}
		}
		
		
		
		
		//SetupVulkanWindow(&g_MainWindowData,m_Surface,m_ViewportWidth, m_ViewportHeight);

		ImGui::CreateContext();
		// Setup Dear ImGui style
		//ImGui::StyleColorsDark();
		ImGui::StyleColorsLight();

		// Setup Platform/Renderer backends
		bool result =ImGui_ImplGlfw_InitForVulkan(m_Window->GetRaw(), true);
		//this initializes imgui for Vulkan
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = m_Instance->GetInstance();
		init_info.PhysicalDevice = m_Device->GetDevicePhys();
		init_info.Device = m_Device->GetDevice();
		init_info.Queue = m_Device->GetGraphicsQueue();
		init_info.DescriptorPool = imguiData.g_DescriptorPool;
		init_info.MinImageCount = m_Swapchain->m_SwapchainFrames.size();
		init_info.ImageCount = m_Swapchain->m_SwapchainFrames.size();
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		result = ImGui_ImplVulkan_Init(&init_info, g_MainWindowData.RenderPass);

		

		auto commandBuffer = CommandBuffer::CreateBuffer(imguiData.g_CommandPool, vk::CommandBufferLevel::ePrimary);
		commandBuffer.BeginTransfering();

		result =  ImGui_ImplVulkan_CreateFontsTexture((VkCommandBuffer)commandBuffer.GetCommandBuffer());
		commandBuffer.EndTransfering();
		commandBuffer.SubmitSingle();
		commandBuffer.Free();


		
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	



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
		auto frameAmount = m_Swapchain->m_SwapchainFrames.size();
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
		auto extent = m_Swapchain->m_SwapchainExtent;
		auto swapchainFormat = m_Swapchain-> m_SwapchainFormat;
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
		//glm::vec3 eye = { 1.0f, 0.0f, 5.0f };
		//glm::vec3 center = { 0.5f, -1.0f, 0.0f };
		//glm::vec3 up = { 0.0f, 0.0f, 1.0f };
		//cameraView = glm::lookAt(eye,center,up);
		//ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//ubo.view = glm::lookAt(glm::vec3(-2.0f, 0.0f, 4.0f), glm::vec3(0.5f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		////ubo.proj = glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.0001f, 10.0f);
		ubo.playerPos = glm::vec4{ m_App->GetCamera()->m_Position ,0};
		ubo.view = cameraView;
		ubo.proj = cameraProj;
		ubo.time = exeTime;
		//auto model = glm::mat4(1.f);
		//glm::mat4 blenderToLH = glm::mat4(1.0f);
		//blenderToLH[2][2] = -1.0f;  // Flip Z-axis
		//blenderToLH[3][2] = 1.0f;
		////model = blenderToLH * model;
		//// blender: z  is up, y is forward
		//model = glm::rotate(model,glm::radians(-90.f) , glm::vec3(1, 0, 0));
		//model = glm::rotate(model,glm::radians(90.f) , glm::vec3(0, 0, 1));
		//ubo.model = model;
		auto extent = m_Swapchain->m_SwapchainExtent;
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

			
			for (int i = 0; i < m_ComputeCommandBuffer.size();i++)
			{
				m_RenderCommandBuffer[i].Free();
				m_ComputeCommandBuffer[i].Free();
				m_TransferCommandBuffer[i].Free();
			};
			m_CommandPoolManager->FreePool(m_FrameCommandPool);
			m_CommandPoolManager->FreePool(m_TracyCommandPool);
		
	}


	void Renderer::RecordCommandBuffer(uint32_t imageIndex,vk::RenderPass& renderPass,vk::Pipeline& pipeline, vk::PipelineLayout& pipelineLayout, int instances)
	{

		
	}

	
	Renderer* Renderer::Instance()
	{
		static Renderer renderer ;
		return &renderer;
	}
	void Renderer::UpdateTexture()
	{

		auto device = m_Device->GetDevice();
		if (m_ImageSelected->m_ImageLayout != vk::ImageLayout::eGeneral)
		{
			auto cmdBuffer = m_ComputeCommandBuffer[currentFrame].BeginTransfering();


			m_ComputeCommandBuffer[currentFrame].ChangeImageLayout(m_ImageSelected.get(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral);
			m_ComputeCommandBuffer[currentFrame].EndTransfering();
			m_ComputeCommandBuffer[currentFrame].SubmitSingle();

		}
		auto cmdBuffer = m_ComputeCommandBuffer[currentFrame].BeginTransfering();




		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline->m_Pipeline);
		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_ComputePipeline->m_PipelineLayout, 0, 1, &m_DescriptorSets[currentFrame], 0, 0);
		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_ComputePipeline->m_PipelineLayout, 1, 1, &m_DescriptorSetSelected, 0, 0);
		float invocations = 256;
		int localSize = 8;


		vkCmdDispatch(cmdBuffer, invocations / localSize, invocations / localSize, 1);

		m_ComputeCommandBuffer[currentFrame].ChangeImageLayout(m_ImageSelected.get(), vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal);
		m_ComputeCommandBuffer[currentFrame].EndTransfering();
		m_ComputeCommandBuffer[currentFrame].SubmitSingle();


		device.waitIdle();
	}
	void Renderer::UpdateBuffer()
	{
		auto bufferSize = MAX_POINTS * sizeof(glm::vec2);
		{
			SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(bufferSize);




			m_TransferCommandBuffer[0].BeginTransfering();
			m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_ShaderStorageBuffer.get(), (void*)m_ClickPoints.data(), bufferSize);
			m_TransferCommandBuffer[0].EndTransfering();
			m_TransferCommandBuffer[0].SubmitSingle();


			m_Device->UpdateDescriptorSet(m_DescriptorSetSelected, 1, 1, *m_ShaderStorageBuffer, vk::DescriptorType::eStorageBuffer);
		}
	}
	void Renderer::Render(float deltaTime)
	{
		exeTime += deltaTime;
		
		

		{

			ZoneScopedN("Waiting for fence");
			m_Device->GetDevice().waitForFences(m_InFlightFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
		}
		m_Device->GetDevice().resetFences(m_InFlightFence);

		uint32_t imageIndex;
		auto swapchain = m_Swapchain->m_Swapchain;
		{
			ZoneScopedN("Acquiring new Image");
			m_Device->GetDevice().acquireNextImageKHR(swapchain, UINT64_MAX, m_ImageAvailableSemaphore, nullptr, &imageIndex);
		}
		{
			ZoneScopedN("Updating uniform buffer");

			UpdateUniformBuffer(imageIndex);
		}
	
			vk::Semaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
			vk::Semaphore signalSemaphores[] = { m_RenderFinishedSemaphore };

		auto& renderCommandBuffer = m_RenderCommandBuffer[imageIndex];
		{
			ZoneScopedN("Sumbit render commands");
			renderCommandBuffer.BeginRendering();

			

			vk::CommandBufferBeginInfo beginInfo = {};

			auto commandBuffer = m_RenderCommandBuffer[imageIndex].GetCommandBuffer();
			auto amount = m_IndexBuffer->GetIndexAmount();







			commandBuffer.begin(beginInfo);
			{



				TracyVkZone(ctx, commandBuffer, "Rendering ");



				
				m_RenderCommandBuffer[imageIndex].BeginRenderPass(&m_TerrainPipeline->m_RenderPass, &m_Swapchain->m_SwapchainFrames[imageIndex].framebuffer, &m_Swapchain->m_SwapchainExtent);
				vk::Viewport viewport;
				viewport.x = 0;
				viewport.y = 0;
				viewport.minDepth = 0;
				viewport.maxDepth = 1;
				viewport.height = m_ViewportHeight;
				viewport.width = m_ViewportWidth;

				vk::Rect2D scissors;
				scissors.offset = vk::Offset2D{ (uint32_t)0,(uint32_t)0 };
				scissors.extent = vk::Extent2D{ (uint32_t)m_ViewportWidth,(uint32_t)m_ViewportHeight };



				

				commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_TerrainPipeline->m_PipelineLayout, 0, m_DescriptorSets[imageIndex], nullptr);
				commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_TerrainPipeline->m_PipelineLayout, 1, m_DescriptorSetTex, nullptr);
			

				commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_TerrainPipeline->m_Pipeline);
				vk::DeviceSize offsets[] = { 0 };

				{
					vk::Buffer vertexBuffers[] = { m_ModelBuffer->GetBuffer() };
					commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

				}

				


				commandBuffer.setViewport(0, 1, &viewport);
				commandBuffer.setScissor(0, 1, &scissors);

				commandBuffer.bindIndexBuffer(m_IndexBuffer->GetBuffer(), 0, m_IndexBuffer->GetIndexType());
				commandBuffer.drawIndexed(static_cast<uint32_t>(amount),1, 0, 0, 0);

			
			
			}
			m_RenderCommandBuffer[imageIndex].EndRenderPass();
			TracyVkCollect(ctx, commandBuffer);
			commandBuffer.end();
			renderCommandBuffer.EndRendering();

		}


		

		
			
#if IMGUI_ENABLED
		
			RenderImGui(imageIndex);

#endif
			if (Input::IsKeyTyped(VS_KEY_V))
			{
				// find mouse pos in world coordinates
				float scale = 100;
				auto  mousePos = Input::GetMousePos();
				auto mouseScreenPos = glm::vec2{ std::get<0>(mousePos),std::get<1>(mousePos) };
				glm::vec3 ndc;
				glm::vec2 screenSize ={ m_App->m_ScreenWidth, m_App->m_ScreenHeight};
				ndc.x = (2.0f * mouseScreenPos.x / screenSize.x) - 1.0f;
				ndc.y = -(1.0f - (2.0f * mouseScreenPos.y / screenSize.y));
				
				auto vertPos = glm::vec4(0.5 * scale, 0, -0.5 * scale, 1);

				float planeDepthWorld = 0.934;
				float ndcZNear = 10;
				float ndcZFar = 10000;


				ndc.z = (planeDepthWorld - ndcZNear) / (ndcZFar - ndcZNear);
				auto inverseProj = glm::inverse(m_App->m_Camera->GetProj());
				auto inverseView= glm::inverse(m_App->m_Camera->GetView());
				auto rayEye =(inverseProj * glm::vec4{ ndc.x,ndc.y,1,1 });
				
				rayEye.z = 1;
				rayEye.w = 0;
				glm::vec3 rayDir = glm::normalize(glm::vec3(inverseView * rayEye));
			//	rayDir.z *= -1;
				glm::vec3 cameraPos = glm::vec3(inverseView[3]);
				std::cout << "ndc  " << ndc.x << " " << ndc.y << " " << ndc.z << std::endl;
				glm::vec3 rayOrigin = cameraPos;
				glm::vec3 intersectionPoint = {0,0,0};
				std::cout << "ray dir " << rayDir.x << " " << rayDir.y << " " << rayDir.z << std::endl;
				// works only  if camera doesn
				auto RayIntersectObjects = [](glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 planePoint, glm::vec3& intersectionPoint)
				{
					// Calculate the dot product of the ray direction and the plane's normal.
					glm::vec3 planeNormal = {0,1,0};
					float denominator = glm::dot(planeNormal, rayDirection);
					// Check if the ray and the plane are not parallel (denominator is not close to zero).
					if (glm::abs(denominator) > 1e-6) {
						// Calculate the vector from the ray's origin to a point on the plane.
						glm::vec3 rayToPlane = planePoint - rayOrigin;

						// Calculate the distance along the ray where it intersects the plane.
						float t = glm::dot(rayToPlane, planeNormal) / denominator;

						// Check if the intersection point is in front of the ray's origin.
						if (t >= 0.0f) {
							// Calculate the intersection point using the ray's equation: rayOrigin + t * rayDirection.
							intersectionPoint = rayOrigin + t * rayDirection;

							std::cout << "intersection " << intersectionPoint.x  << " "  << intersectionPoint.y  << " " << intersectionPoint.z << std::endl;
							return true; // Intersection occurred.
						}
					}
					return false;
				};

				bool hit = RayIntersectObjects(rayOrigin, rayDir, vertPos, intersectionPoint);
				/*worldSpace = worldSpace / worldSpace.w;

				*/
				auto localPosition = (intersectionPoint- glm::vec3(vertPos));
				glm::vec3 uAxis = glm::vec3(-1, 0, 0);
				glm::vec3 vAxis = glm::vec3(0, 0, -1);

				glm::vec2 uvSpace = glm::vec2{ glm::dot(glm::vec3{localPosition} , uAxis),glm::dot(glm::vec3{localPosition}, vAxis) }/100.f;
				uvSpace.y *= -1;
				//uvSpace = glm::normalize(uvSpace);
				std::cout << "uv space " << uvSpace.x << " " <<  uvSpace.y << std::endl;
				//std::cout <<  "local pos " << localPosition.x << " " << localPosition.y << " " << localPosition.z << std::endl;
				//std::cout << "ray direction "  << rayDir.x << " " << rayDir.y << " " << rayDir.z << std::endl;
				// update vector of positions
				m_ClickPoints[nextPoint++] = (uvSpace);
				nextPoint %= MAX_POINTS;


				UpdateBuffer();
				


			}

				UpdateTexture();

			vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
			vk::SubmitInfo submitInfo = {};

			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			const std::vector<vk::CommandBuffer> commandBuffers = { renderCommandBuffer.GetCommandBuffer(), g_MainWindowData.Frames[imageIndex].CommandBuffer };
			// std::vector<vk::CommandBuffer> commandBuffers = { renderCommandBuffer.GetCommandBuffer() };

			submitInfo.commandBufferCount = commandBuffers.size();
			submitInfo.pCommandBuffers = commandBuffers.data();

			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;
			auto device = RenderContext::GetDevice();
			device->GetGraphicsQueue().submit(submitInfo, m_InFlightFence);
		

		

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


		currentFrame = (currentFrame + 1) % m_Swapchain->m_SwapchainFrames.size();

		
		
		FrameMark;
	}


	//On tile - based - renderer, which is pretty much anything on mobile,
	//using input attachments is faster than the traditional multi - pass approach as pixel reads are fetched from tile memory instead of mainframebuffer, 
	//so if you target the mobile market it’s always a good idea to use input attachments instead of multiple passes when possible.








	/*No.The attachment reference layout tells Vulkan what layout to transition the image to at the beginning of the subpass for which this reference is defined.Or more to the point, it is the layout which the image will be in for the duration of the subpass.
	* 

		The first transition for an attached image of a render pass will be from the initialLayout 
		for the render pass to the reference layout for the first subpass that uses the image.
		The last transition for an attached image will be from reference layout of the final subpass 
		that uses the attachment to the finalLayout for the render pass.*/



	// create a builder for render pass
	static vk::RenderPass MakeRenderPass(vk::Device device, vk::Format swapchainImageFormat, vk::Format depthFormat)
	{

		auto samples = RenderContext::GetDevice()->GetSamples();
		//Define a general attachment, with its load/store operations
		vk::AttachmentDescription msaaAttachment = AttachmentDescription( swapchainImageFormat,samples, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
	


		vk::AttachmentReference msaaAttachmentRef = {};
		msaaAttachmentRef.attachment = 0;
		msaaAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;



		vk::AttachmentDescription depthAttachment = AttachmentDescription(depthFormat, samples, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);


		

		vk::AttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		//Define a general attachment, with its load/store operations
		vk::AttachmentDescription colorAttachmentResolve =
			AttachmentDescription(swapchainImageFormat, vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
	

		//Declare that attachment to be color buffer 0 of the framebuffer
		vk::AttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 2;
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;


		//Renderpasses are broken down into subpasses, there's always at least one.



		vk::SubpassDescription subpass = SubpassDescription(1, &msaaAttachmentRef, &colorAttachmentRef, &depthAttachmentRef);
		


		
		std::vector<vk::SubpassDescription> subpasses = { subpass  };


		vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL,0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eNone, vk::PipelineStageFlagBits::eColorAttachmentOutput,vk::AccessFlagBits::eColorAttachmentWrite);


		std::vector<vk::SubpassDependency> dependencies = { dependency0  };
		

		//Now create the renderpass
		vk::RenderPassCreateInfo renderpassInfo = {};

		std::vector<vk::AttachmentDescription> attachments = { msaaAttachment,depthAttachment,colorAttachmentResolve };
		// to be able to map NDC to screen coordinates - Viewport ans Scissors Transform
		renderpassInfo.flags = vk::RenderPassCreateFlags();
		renderpassInfo.attachmentCount = attachments.size();
		renderpassInfo.pAttachments = attachments.data();
		renderpassInfo.subpassCount = subpasses.size();
		renderpassInfo.pSubpasses = subpasses.data();
		renderpassInfo.dependencyCount = dependencies.size();
		renderpassInfo.pDependencies = dependencies.data();

		try {
			return device.createRenderPass(renderpassInfo);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to create renderpass!");
		}
	}
	void Renderer::CreateFramebuffers()
	{
		auto& frames = m_Swapchain->m_SwapchainFrames;
		auto swapChainExtent = m_Swapchain->m_SwapchainExtent;
		for (int i = 0; i < frames.size(); ++i) {

			std::vector<vk::ImageView> attachments = {
				m_MsaaImageView,
				frames[i].imageDepthView,
				frames[i].imageView,
			};

			vk::FramebufferCreateInfo framebufferInfo;
			framebufferInfo.flags = vk::FramebufferCreateFlags();
			framebufferInfo.renderPass = m_TerrainPipeline->m_RenderPass;
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
		auto swapchainFormat = m_Swapchain->m_SwapchainFormat;
		auto swapChainExtent = m_Swapchain->m_SwapchainExtent;


		 m_RenderPass = MakeRenderPass(m_Device->GetDevice(), swapchainFormat, m_Swapchain->m_SwapchainFrames[0].depthFormat);


		// terrain pipeline
		{
			GraphicsPipelineSpecification specs;

			specs.device = m_Device->GetDevice();

			specs.vertexFilepath = BASE_SPIRV_OUTPUT + "default.spvV";
			specs.fragmentFilepath = BASE_SPIRV_OUTPUT + "dos2.spvF";
			specs.swapchainExtent = swapChainExtent;
			specs.swapchainImageFormat = swapchainFormat;


			std::vector<vk::VertexInputBindingDescription> bindings{ VertexBindingDescription(0,sizeof(Vertex),vk::VertexInputRate::eVertex) };

			std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;

			attributeDescriptions =
			{
				VertexInputAttributeDescription(0,0,vk::Format::eR32G32B32Sfloat,offsetof(Vertex, Position)),
			   VertexInputAttributeDescription(0,1,vk::Format::eR32G32B32A32Sfloat,offsetof(Vertex, Color)),
				VertexInputAttributeDescription(0,2,vk::Format::eR32G32Sfloat,offsetof(Vertex, UV)),

			};

			specs.bindingDescription = bindings;



			specs.attributeDescription = attributeDescriptions;

			auto samples = RenderContext::GetDevice()->GetSamples();
			specs.samples = samples;
			auto pipelineLayouts = std::vector<vk::DescriptorSetLayout>{ m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutTex->GetLayout() };

			specs.descriptorSetLayout = pipelineLayouts;

			



			
			m_TerrainPipeline = Pipeline::CreateGraphicsPipeline(specs, vk::PrimitiveTopology::eTriangleList, m_Swapchain->m_SwapchainFrames[0].depthFormat, m_RenderPass,0,true,m_PolygoneMode);
		
		}
		
		
		
	}

	



	
	void Renderer::RenderImGui(int imageIndex)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		bool show_demo_window = false;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);



		ImGui::Begin("Surface paramentrs", &show_another_window);
		if (ImGui::Button("clear points"))
		{
			for (int i = 0; i < m_ClickPoints.size(); i++)
			{
				m_ClickPoints[i] = { -1,-1 };
			}
			nextPoint = 0;
			UpdateBuffer();

		}


		
	

		ImGui::End();
		m_IsNewParametrs |= m_IsResized;

		// Rendering
		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();
		const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
		auto wd = &g_MainWindowData;
		if (!is_minimized)
		{
			wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
			wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
			wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
			wd->ClearValue.color.float32[3] = clear_color.w;

			// frame render
			{


				ImGui_ImplVulkanH_Frame* fd = &wd->Frames[imageIndex];
				{

				}
				{
					vkResetCommandBuffer(fd->CommandBuffer, 0);
					VkCommandBufferBeginInfo info = {};
					info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
					info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
					vkBeginCommandBuffer(fd->CommandBuffer, &info);
				}
				{
					VkRenderPassBeginInfo info = {};
					info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
					info.renderPass = wd->RenderPass;
					info.framebuffer = fd->Framebuffer;
					info.renderArea.extent.width = wd->Width;
					info.renderArea.extent.height = wd->Height;
					info.clearValueCount = 1;
					info.pClearValues = &wd->ClearValue;
					vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
				}

				// Record dear imgui primitives into command buffer
				ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

				// Submit command buffer
				vkCmdEndRenderPass(fd->CommandBuffer);
				vkEndCommandBuffer(fd->CommandBuffer);
				{

				}
			}


		}




	}
	void Renderer::CleanUpImGui()
	{
#if IMGUI_ENABLED
		m_CommandPoolManager->FreePool(imguiData.g_CommandPool);
		m_Device->GetDevice().destroyRenderPass(imguiData.g_RenderPass);
		
		auto device = RenderContext::GetDevice()->GetDevice();
		for (int i = 0; i < g_MainWindowData.ImageCount; i++)
		{
			device.destroyFramebuffer(g_MainWindowData.Frames[i].Framebuffer);
		}
		device.destroyDescriptorPool(imguiData.g_DescriptorPool);
		delete g_MainWindowData.Frames;
		
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();


#endif

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


	

	Renderer::~Renderer()
	{


		auto device = m_Device->GetDevice();
		device.waitIdle();
		
		CleanUpImGui();

		Shutdown();

		TracyVkDestroy(ctx)
		if (m_ModelBuffer != nullptr)
		{
			m_ModelBuffer.reset();
			m_IndexBuffer.reset();

		}

		

		m_Device->GetDevice().freeMemory(m_MsaaImageMemory);
		m_Device->GetDevice().destroyImage(m_MsaaImage);
		m_Device->GetDevice().destroyImageView(m_MsaaImageView);



		for (size_t i = 0; i < m_Swapchain->m_SwapchainFrames.size(); i++) {
			delete m_UniformBuffers[i];
		}

		m_ShaderStorageBuffer.reset();
		
		m_DescriptorPool.reset();
		m_DescriptorPoolSelected.reset();
		m_DescriptorPoolTex.reset();
		m_Device->GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayout->GetLayout());
		m_Device->GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayoutSelected->GetLayout());
		m_Device->GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayoutTex->GetLayout());
	

	
		m_ImageSelected.reset();
		m_Image.reset();
	
		
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
		device.destroyRenderPass(m_RenderPass);
		m_TerrainPipeline.reset();
		m_ComputePipeline.reset();
		m_Swapchain.reset();


		m_CommandPoolManager->Release();
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
