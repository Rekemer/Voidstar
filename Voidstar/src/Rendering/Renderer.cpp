
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
#include  "Binder.h"
#include  "Generation.h"
#include"Settings.h"



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

	
	QuadData quad;

	TracyVkCtx ctx;
	vk::ShaderModule CreateModule(std::string filename, vk::Device device);
const int QUAD_AMOUNT = 700;

	size_t currentFrame = 0;
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
				auto extension = shader.path().extension().string();
				auto shaderString = shader.path().filename().string();
				std::string shaderPath = shader.path().string();

				std::string command = "";
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




	

	

	
	
	void Renderer::CreateLayouts()
	{
		for (auto [key, value] : m_Bindings)
		{
			m_Layout[key] = DescriptorSetLayout::Create(value);
		}
	}
	void Renderer::AllocateSets()
	{
		std::variant<vk::DescriptorSet, std::vector<vk::DescriptorSet>> a;
		a = std::vector<vk::DescriptorSet>();
		for (auto [key, value] : m_Layout)
		{
			std::vector<vk::DescriptorSetLayout> layouts(m_SetsAmount[key], value->GetLayout());
			auto sets = m_UniversalPool->AllocateDescriptorSets(m_SetsAmount[key], layouts.data());
			if (m_SetsAmount[key] == 1)
			{
				m_Sets[key] = sets[0];
			}
			else
			{
				m_Sets[key] = sets;
			}
			
		}
		

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
		// TODO: �������� ����� �������� return
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
	}

	void Renderer::DrawBatch(vk::CommandBuffer& commandBuffer)
	{
		vk::DeviceSize offsets[] = { 0 };

		{
			vk::Buffer vertexBuffers[] = { m_QuadBufferBatch->GetBuffer() };
			commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

		}
		commandBuffer.bindIndexBuffer(m_QuadBufferBatchIndex->GetBuffer(), 0, m_QuadBufferBatchIndex->GetIndexType());
		commandBuffer.drawIndexed(m_QuadIndex, 1, 0, 0, 0);
	
	}

	void UpdateVertex(Vertex*& vertex, glm::vec3 position, glm::vec4& color, glm::mat4& world, int vertIndex)
	{
		vertex->Position = world * glm::vec4{ position,1 };
		vertex->UV = quad.verticies[vertIndex].UV;
		vertex->Color = color;
	};
	void UpdateVerticies(Vertex*& vertex, std::vector<Vertex>& verticies)
	{
		UpdateVertex(vertex,verticies[0].Position, verticies[0].Color, glm::identity<glm::mat4>(),0);
		vertex++;
		UpdateVertex(vertex,verticies[2].Position, verticies[2].Color, glm::identity<glm::mat4>(),2);
		vertex++;
		UpdateVertex(vertex,verticies[3].Position, verticies[3].Color, glm::identity<glm::mat4>(),3);
		vertex++;
		UpdateVertex(vertex,verticies[1].Position, verticies[1].Color, glm::identity<glm::mat4>(),1);
		vertex++;

	};
	void Renderer::DrawTxt(vk::CommandBuffer commandBuffer, std::string_view str, glm::vec2 pos, std::map< unsigned char, Character>& characters)
	{
		assert(false);
		float scale = 0.5;
		auto offset = pos;
		for (auto e : str)
		{

			if (e == '\n')
			{
				offset.x = pos.x;
				pos.y -= Character::lineSpacing/64;
				continue;
			}
			else if (e == ' ')
			{
				offset.x += 15;
				continue;

			}
			else if (e == '\t')
			{
				offset.x += 30;
				continue;

			}
			if (characters.find(e) == characters.end()) continue;
		//auto& characterData = characters.at(e);
		//offset.x = offset.x + characterData.Bearing.x* scale;
		//// to account for letter like p and q
		//offset.y = pos.y - ( characterData.Size.y - characterData.Bearing.y)* scale;
		//glm::vec4 color{ 1 };
		//glm::mat4 world{ 1 };
		//// left bottom
		//UpdateVertex(m_BatchQuad, verticies, color, world, 0);
		//m_BatchQuad->u = characterData.minUv.x;
		//m_BatchQuad->v = characterData.maxUv.y;
		//m_BatchQuad->x = offset.x;
		//m_BatchQuad->y = offset.y;
		//m_BatchQuad->z = 0;
		//
		//m_BatchQuad++;
		//// right bottom
		//m_BatchQuad->u = characterData.maxUv.x;
		//m_BatchQuad->v = characterData.maxUv.y;
		//m_BatchQuad->x = offset.x + characterData.Size.x* scale;
		//m_BatchQuad->y = offset.y;
		//m_BatchQuad->z = 0;
		//m_BatchQuad++;
		//// right top
		//m_BatchQuad->u = characterData.maxUv.x;
		//m_BatchQuad->v = characterData.minUv.y;
		//m_BatchQuad->x = offset.x + characterData.Size.x * scale;
		//m_BatchQuad->y = offset.y + characterData.Size.y * scale;
		//m_BatchQuad->z = 0;
		//m_BatchQuad++;
		//
		//
		//// left top
		//m_BatchQuad->u = characterData.minUv.x;
		//m_BatchQuad->v = characterData.minUv.y;
		//m_BatchQuad->x = offset.x;
		//m_BatchQuad->y = offset.y + characterData.Size.y * scale;
		//m_BatchQuad->z = 0;
		//m_BatchQuad++;
		//
		//
		//offset.x += characterData.Advance / 64.f* scale;
		//m_QuadIndex += 6;
		}
		DrawBatch(commandBuffer);
	}
	void Renderer::DrawQuadScreen(vk::CommandBuffer commandBuffer)
	{
		assert(false);
		vk::DeviceSize offsets[] = { 0 };

		{
			vk::Buffer vertexBuffers[] = { m_QuadBufferBatch->GetBuffer() };
			commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

		}
		commandBuffer.bindIndexBuffer(m_QuadBufferBatchIndex->GetBuffer(), 0, m_QuadBufferBatchIndex->GetIndexType());
		commandBuffer.drawIndexed(5*6, 1, 0, 0,0);
	}
	void Renderer::DrawQuad(glm::mat4& world, glm::vec4 color)
	{
		auto& verticies = quad.verticies;
		// left bottom
		UpdateVertex(m_BatchQuad,verticies[0].Position,color,world,0);
		m_BatchQuad++;
		// right bottom
		UpdateVertex(m_BatchQuad,verticies[2].Position,color,world,2);
		m_BatchQuad++;
		// right top
		UpdateVertex(m_BatchQuad, verticies[3].Position, color, world, 3);
		m_BatchQuad++;


		// left top
		UpdateVertex(m_BatchQuad, verticies[1].Position, color, world, 1);
		m_BatchQuad++;

		m_QuadIndex += 6;
	}

	void Renderer::DrawQuad(std::vector<Vertex>& verticies)
	{

		UpdateVerticies(m_BatchQuad, verticies);
		m_QuadIndex += 6;

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
		

		std::vector<vk::DescriptorPoolSize> pool_sizes =
		{
			{ vk::DescriptorType::eCombinedImageSampler, 10 },
			{ vk::DescriptorType::eStorageImage, 10 },
			{ vk::DescriptorType::eStorageBuffer, 10 },
			{ vk::DescriptorType::eInputAttachment, 10 },
			{ vk::DescriptorType::eUniformBuffer, 10 },
		};

		m_UniversalPool = DescriptorPool::Create(pool_sizes, 10);

		CreateSyncObjects();
		m_Swapchain->CreateMSAAFrame();



		auto commandBufferInit = [this]()
		{
			m_FrameCommandPool = Renderer::Instance()->GetCommandPoolManager()->GetFreePool();
			m_RenderCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, 3);
			m_TransferCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, 3);
			m_ComputeCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, 3);
		};
		commandBufferInit();
		quad = GeneratePlane(1);
		auto& verticies = quad.verticies;
		auto& indices = quad.indicies;
		
		auto indexSize = SizeOfBuffer(indices.size(), indices[0]);
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


	void Renderer::UserInit()
	{
		m_UserFunctions.bindingsInit();
		CreateLayouts();
		AllocateSets();
		m_UserFunctions.bufferInit();
		m_UserFunctions.loadTextures();
		m_UserFunctions.bindResources();
		m_UserFunctions.createPipelines();
		m_UserFunctions.createFramebuffer();

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




	void Renderer::RecreateSwapchain()
	{
		m_ViewportWidth = 0;
		m_ViewportHeight = 0;
		while (m_ViewportWidth == 0 || m_ViewportHeight == 0) {
			glfwGetFramebufferSize(m_Window->GetRaw(), &m_ViewportWidth, &m_ViewportHeight);
			glfwWaitEvents();
		}


		m_Device->GetDevice().waitIdle();
		m_Swapchain->CleanUp();


		SwapChainSupportDetails support;
		support.devcie = m_Device;
		support.surface = &m_Surface;
		support.capabilities = m_Device->GetDevicePhys().getSurfaceCapabilitiesKHR(m_Surface);
		support.formats = m_Device->GetDevicePhys().getSurfaceFormatsKHR(m_Surface);
		support.presentModes = m_Device->GetDevicePhys().getSurfacePresentModesKHR(m_Surface);
		support.viewportWidth = m_ViewportWidth;
		support.viewportHeight = m_ViewportHeight;
		m_Swapchain = Swapchain::Create(support);
		auto& camera = m_App->GetCamera();
		camera->UpdateProj(m_ViewportWidth, m_ViewportHeight);
	}

	void Renderer::Shutdown()
	{

			
			


			auto device = m_Device->GetDevice();
			device.waitIdle();

			CleanUpImGui();


			TracyVkDestroy(ctx)

			m_CommandPoolManager->FreePool(m_TracyCommandPool);

			m_UserFunctions.cleanUp();
			m_Swapchain->CleanUp();


			for (int i = 0; i < m_ComputeCommandBuffer.size(); i++)
			{
				m_RenderCommandBuffer[i].Free();
				m_ComputeCommandBuffer[i].Free();
				m_TransferCommandBuffer[i].Free();
			};

			Renderer::Instance()->GetCommandPoolManager()->FreePool(m_FrameCommandPool);




			m_UniversalPool.reset();
			CleanUpLayouts();


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


			m_CommandPoolManager->Release();
			m_Device->GetDevice().destroy();

			m_Instance->GetInstance().destroySurfaceKHR(m_Surface);
			m_Instance->GetInstance().destroyDebugUtilsMessengerEXT(m_DebugMessenger, nullptr, m_Dldi);
			m_Instance->GetInstance().destroy();


		
	}


	void Renderer::RecordCommandBuffer(uint32_t imageIndex,vk::RenderPass& renderPass,vk::Pipeline& pipeline, vk::PipelineLayout& pipelineLayout, int instances)
	{

		
	}

	
	Renderer* Renderer::Instance()
	{
		static Renderer renderer ;
		return &renderer;
	}
	void Renderer::WaitFence()
	{
		{

			ZoneScopedN("Waiting for fence");
			m_Device->GetDevice().waitForFences(m_InFlightFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
		}
		m_Device->GetDevice().resetFences(m_InFlightFence);
	}
	void Renderer::Render(float deltaTime,Camera& camera)
	{
		auto exeTime = m_App->GetExeTime();
		
		

		{

		//	ZoneScopedN("Waiting for fence");
		//	m_Device->GetDevice().waitForFences(m_InFlightFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
		//m_Device->GetDevice().resetFences(m_InFlightFence);
		}

		uint32_t imageIndex;

		auto swapchain = m_Swapchain->m_Swapchain;
		{
			ZoneScopedN("Acquiring new Image");
			m_Device->GetDevice().acquireNextImageKHR(swapchain, UINT64_MAX, m_ImageAvailableSemaphore, nullptr, &imageIndex);
		}
		
			m_UserFunctions.postRenderCommands(imageIndex, camera);
	
			auto signalSemaphore = m_UserFunctions.submitRenderCommands(imageIndex, camera,m_ImageAvailableSemaphore, m_InFlightFence);
			vk::Semaphore waitSemaphore[] = { signalSemaphore };
	
			
#if IMGUI_ENABLED
		
			RenderImGui(imageIndex);
			commandBuffers.push_back(g_MainWindowData.Frames[imageIndex].CommandBuffer);
#endif
			/*
			 std::vector<vk::CommandBuffer> commandBuffers = { renderCommandBuffer.GetCommandBuffer() };
			vk::Semaphore waitSemaphores[] = { m_ImageAvailableSemaphore };

			vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
			vk::SubmitInfo submitInfo = {};

			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;


			submitInfo.commandBufferCount = commandBuffers.size();
			submitInfo.pCommandBuffers = commandBuffers.data();

			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;
			auto device = RenderContext::GetDevice();
			device->GetGraphicsQueue().submit(submitInfo, m_InFlightFence);*/
			

		

		vk::PresentInfoKHR presentInfo = {};
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = waitSemaphore;

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


	

	


	

	



	
	void Renderer::Flush(std::vector<vk::CommandBuffer> commandBuffers)
	{
		assert(false);
		

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
