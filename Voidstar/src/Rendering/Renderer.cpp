
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
	#define	IMGUI_ENABLED 1
	size_t currentFrame = 0;
	static float exeTime = 24;
	// noise
	const float noiseTextureWidth = 450.f;
	const float noiseTextureHeight = 450.f;
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


	

	void Renderer::CreateComputePipeline()
	{

		{

			std::vector<vk::DescriptorPoolSize> poolSizes;
			{
				vk::DescriptorPoolSize poolSize;
				poolSize.type = vk::DescriptorType::eStorageImage;
				poolSize.descriptorCount = 1;
				poolSizes.emplace_back(poolSize);
			}

			{
				vk::DescriptorPoolSize poolSize;
				poolSize.type = vk::DescriptorType::eUniformBuffer;
				poolSize.descriptorCount = 1;
				poolSizes.emplace_back(poolSize);
			}
		
			m_DescriptorPoolNoise = DescriptorPool::Create(poolSizes, 1);

			std::vector<vk::DescriptorSetLayout> layouts;
			layouts.resize(1);


			vk::DescriptorSetLayoutBinding layoutBinding1;
			layoutBinding1.binding = 0;
			layoutBinding1.descriptorType = vk::DescriptorType::eStorageImage;
			layoutBinding1.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eTessellationEvaluation |
				vk::ShaderStageFlagBits::eCompute ;
			layoutBinding1.descriptorCount = 2;

			vk::DescriptorSetLayoutBinding layoutBinding2;
			layoutBinding2.binding = 1;
			layoutBinding2.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			layoutBinding2.stageFlags = vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eVertex |
				vk::ShaderStageFlagBits::eTessellationEvaluation;
			layoutBinding2.descriptorCount = 2;

			vk::DescriptorSetLayoutBinding layoutBinding3;
			layoutBinding3.binding = 2;
			layoutBinding3.descriptorType = vk::DescriptorType::eUniformBuffer;
			layoutBinding3.stageFlags = vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eVertex |
				vk::ShaderStageFlagBits::eTessellationEvaluation | vk::ShaderStageFlagBits::eFragment;
			layoutBinding3.descriptorCount = 1;

			vk::DescriptorSetLayoutBinding layoutBinding4;
			layoutBinding4.binding = 3;
			layoutBinding4.descriptorType = vk::DescriptorType::eStorageImage;
			layoutBinding4.stageFlags = vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eVertex |
				vk::ShaderStageFlagBits::eTessellationEvaluation;
			layoutBinding4.descriptorCount = 1;

			vk::DescriptorSetLayoutBinding layoutBinding5;
			layoutBinding5.binding = 4;
			layoutBinding5.descriptorType = vk::DescriptorType::eStorageImage;
			layoutBinding5.stageFlags = vk::ShaderStageFlagBits::eCompute;
			layoutBinding5.descriptorCount = 1;

			vk::DescriptorSetLayoutBinding layoutBinding6;
			layoutBinding6.binding = 5;
			layoutBinding6.descriptorType = vk::DescriptorType::eStorageImage;
			layoutBinding6.stageFlags = vk::ShaderStageFlagBits::eCompute;
			layoutBinding6.descriptorCount = 1;



			std::vector<vk::DescriptorSetLayoutBinding> layoutBindings{ layoutBinding1,layoutBinding2,layoutBinding3,layoutBinding4,layoutBinding5,layoutBinding6 };

			m_DescriptorSetLayoutNoise = DescriptorSetLayout::Create(layoutBindings);

			layouts[0] = m_DescriptorSetLayoutNoise->GetLayout();
			m_DescriptorSetNoise = m_DescriptorPoolNoise->AllocateDescriptorSets(1, layouts.data())[0];
		}


		
		m_NoiseImage = Image::CreateEmptyImage(noiseTextureWidth, noiseTextureHeight, vk::Format::eR8G8B8A8Snorm);
		m_AnimatedNoiseImage = Image::CreateEmptyImage(noiseTextureWidth, noiseTextureHeight, vk::Format::eR8G8B8A8Snorm);
		
		int size = 128;
		m_3DNoiseTexture = Image::CreateEmpty3DImage(size, size, size, vk::Format::eR8G8B8A8Snorm);
		m_3DNoiseTextureLowRes = Image::CreateEmpty3DImage(32, 32, 32, vk::Format::eR8G8B8A8Snorm);
	

		

		{
			vk::DescriptorImageInfo imageDescriptor0;
			imageDescriptor0.imageLayout = vk::ImageLayout::eGeneral;
			imageDescriptor0.imageView = m_NoiseImage->m_ImageView;
			imageDescriptor0.sampler = m_NoiseImage->m_Sampler;
			vk::DescriptorImageInfo imageDescriptor1;
			imageDescriptor1.imageLayout = vk::ImageLayout::eGeneral;
			imageDescriptor1.imageView = m_AnimatedNoiseImage->m_ImageView;
			imageDescriptor1.sampler = m_AnimatedNoiseImage->m_Sampler;
			vk::DescriptorImageInfo imageDescriptor2;
			imageDescriptor2.imageLayout = vk::ImageLayout::eGeneral;
			imageDescriptor2.imageView = m_3DNoiseTexture->m_ImageView;
			imageDescriptor2.sampler = m_3DNoiseTexture->m_Sampler;

			vk::DescriptorImageInfo imageDescriptor3;
			imageDescriptor3.imageLayout = vk::ImageLayout::eGeneral;
			imageDescriptor3.imageView = m_3DNoiseTextureLowRes->m_ImageView;
			imageDescriptor3.sampler = m_3DNoiseTextureLowRes->m_Sampler;
		

			std::vector<vk::DescriptorImageInfo> images{ imageDescriptor0 ,imageDescriptor1};
			m_Device->UpdateDescriptorSet(m_DescriptorSetNoise, 0, images, vk::DescriptorType::eStorageImage);
			m_Device->UpdateDescriptorSet(m_DescriptorSetNoise, 3,1, imageDescriptor2, vk::DescriptorType::eStorageImage);
			m_Device->UpdateDescriptorSet(m_DescriptorSetNoise, 4,1, imageDescriptor3, vk::DescriptorType::eStorageImage);
		


			{
				vk::DescriptorImageInfo imageDescriptor0;
				imageDescriptor0.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				imageDescriptor0.imageView = m_NoiseImage->m_ImageView;
				imageDescriptor0.sampler = m_NoiseImage->m_Sampler;
				vk::DescriptorImageInfo imageDescriptor1;
				imageDescriptor1.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				imageDescriptor1.imageView = m_AnimatedNoiseImage->m_ImageView;
				imageDescriptor1.sampler = m_AnimatedNoiseImage->m_Sampler;
				std::vector<vk::DescriptorImageInfo> images{ imageDescriptor0 ,imageDescriptor1 };
				m_Device->UpdateDescriptorSet(m_DescriptorSetTex, 0, images, vk::DescriptorType::eCombinedImageSampler);

				vk::DescriptorImageInfo imageDescriptor2;
				imageDescriptor2.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				imageDescriptor2.imageView = m_3DNoiseTexture->m_ImageView;
				imageDescriptor2.sampler = m_3DNoiseTexture->m_Sampler;

				vk::DescriptorImageInfo imageDescriptor3;
				imageDescriptor3.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				imageDescriptor3.imageView = m_3DNoiseTextureLowRes->m_ImageView;
				imageDescriptor3.sampler = m_3DNoiseTextureLowRes->m_Sampler;

				vk::DescriptorImageInfo imageDescriptor4;
				imageDescriptor4.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

				m_Device->UpdateDescriptorSet(m_DescriptorSetClouds, 0,1, imageDescriptor2, vk::DescriptorType::eCombinedImageSampler);
				m_Device->UpdateDescriptorSet(m_DescriptorSetClouds, 2,1, imageDescriptor3, vk::DescriptorType::eCombinedImageSampler);
			}
			


		}

		std::string right = "sky/bluecloud_rt.jpg";
		std::string left = "sky/bluecloud_lf.jpg";
		std::string top = "sky/bluecloud_up.jpg";
		std::string down = "sky/bluecloud_dn.jpg";
		std::string forward= "sky/bluecloud_ft.jpg";
		std::string back= "sky/bluecloud_bk.jpg";

		right = "test/skybox/right.jpg";
		left = "test/skybox/left.jpg";
		top = "test/skybox/top.jpg";
		down = "test/skybox/bottom.jpg";
		forward = "test/skybox/front.jpg";
		back = "test/skybox/back.jpg";

		right = "sky2/Daylight Box_Right.bmp";
		left = "sky2/Daylight Box_Left.bmp";
		top = "sky2/Daylight Box_Top.bmp";
		down = "sky2/Daylight Box_Bottom.bmp";
		forward = "sky2/Daylight Box_Front.bmp";
		back = "sky2/Daylight Box_Back.bmp";

		// left x
		//std::vector<std::string> cubemap = { BASE_RES_PATH+ left,
		//BASE_RES_PATH + right,
		//BASE_RES_PATH + down,
		//BASE_RES_PATH + top,
		//BASE_RES_PATH + back, // back -z
		//BASE_RES_PATH + forward }; // forward +z

		 std::vector<std::string> cubemap = {
		 BASE_RES_PATH + right,    // Positive X
		 BASE_RES_PATH + left,     // Negative X
		 BASE_RES_PATH + down,     // Positive Y
		 BASE_RES_PATH + top,      // Negative Y
		 BASE_RES_PATH + forward,  // Positive Z
		 BASE_RES_PATH + back      // Negative Z
		 };
		m_Cubemap = Image::CreateCubemap(cubemap);
		m_Device->UpdateDescriptorSet(m_DescriptorSetSky, 0, 1, *m_Cubemap, vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eCombinedImageSampler);
		m_SnowTex = Image::CreateImage(BASE_RES_PATH + "terrain/snow/Snow_003_COLOR.jpg");
		m_Device->UpdateDescriptorSet(m_DescriptorSetTex,1,1,*m_SnowTex,vk::ImageLayout::eShaderReadOnlyOptimal,vk::DescriptorType::eCombinedImageSampler);
		m_GrassTex = Image::CreateImage(BASE_RES_PATH +"terrain/grass.jpg");
		m_Device->UpdateDescriptorSet(m_DescriptorSetTex,2,1,*m_GrassTex, vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eCombinedImageSampler);
		m_StoneTex = Image::CreateImage(BASE_RES_PATH + "terrain/GroundTex/rock_01_diffuse.jpg");
		m_Device->UpdateDescriptorSet(m_DescriptorSetTex,3,1,*m_StoneTex, vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eCombinedImageSampler);

		m_WaterNormalImage = Image::CreateImage(BASE_RES_PATH + "water/water-normal-1.jpg");
		m_WaterNormalImage2 = Image::CreateImage(BASE_RES_PATH + "water/water-normal-2.jpg");
		{
			vk::DescriptorImageInfo imageDescriptor0;
			imageDescriptor0.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			imageDescriptor0.imageView = m_WaterNormalImage->m_ImageView;
			imageDescriptor0.sampler = m_WaterNormalImage->m_Sampler;
			vk::DescriptorImageInfo imageDescriptor1;
			imageDescriptor1.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			imageDescriptor1.imageView = m_WaterNormalImage2->m_ImageView;
			imageDescriptor1.sampler = m_WaterNormalImage2->m_Sampler;
			std::vector<vk::DescriptorImageInfo> images{ imageDescriptor0 ,imageDescriptor1 };
			m_Device->UpdateDescriptorSet(m_DescriptorSetNoise, 1, images, vk::DescriptorType::eCombinedImageSampler);
		}

		{
			BufferInputChunk inputBuffer;
			inputBuffer.size = sizeof(NoiseData);
			inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			inputBuffer.usage = vk::BufferUsageFlagBits::eUniformBuffer;

			m_NoiseData = new Buffer(inputBuffer);
			m_NoiseDataPtr= m_Device->GetDevice().mapMemory(m_NoiseData->GetMemory(), 0, sizeof(NoiseData));
			memcpy(m_NoiseDataPtr, &noiseData, sizeof(NoiseData));

			m_Device->UpdateDescriptorSet(m_DescriptorSetNoise,2,1, *m_NoiseData, vk::DescriptorType::eUniformBuffer);
			
		}
		
		// create compute layout


		auto device = RenderContext::GetDevice();
		auto computeShaderModule = CreateModule(BASE_SPIRV_OUTPUT + "NoiseTex.spvCmp", device->GetDevice());

		vk::PipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.flags = vk::PipelineShaderStageCreateFlags();
		computeShaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
		computeShaderStageInfo.module = computeShaderModule;
		computeShaderStageInfo.pName = "main";


		std::vector<vk::DescriptorSetLayout> layouts = { m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutNoise->GetLayout() };

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		m_ComputePipelineLayout = device->GetDevice().createPipelineLayout(pipelineLayoutInfo, nullptr);


		vk::ComputePipelineCreateInfo pipelineInfo{};

		pipelineInfo.sType = vk::StructureType::eComputePipelineCreateInfo;
		pipelineInfo.layout = m_ComputePipelineLayout;
		pipelineInfo.stage = computeShaderStageInfo;
		m_ComputePipeline = device->GetDevice().createComputePipeline(nullptr, pipelineInfo).value;



		vkDestroyShaderModule(device->GetDevice(), computeShaderModule, nullptr);


		{

			auto device = RenderContext::GetDevice();
			auto computeShaderModule = CreateModule(BASE_SPIRV_OUTPUT + "NoiseClouds.spvCmp", device->GetDevice());

			vk::PipelineShaderStageCreateInfo computeShaderStageInfo{};
			computeShaderStageInfo.flags = vk::PipelineShaderStageCreateFlags();
			computeShaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
			computeShaderStageInfo.module = computeShaderModule;
			computeShaderStageInfo.pName = "main";


			std::vector<vk::DescriptorSetLayout> layouts = { m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutNoise->GetLayout() };

			vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
			pipelineLayoutInfo.setLayoutCount = layouts.size();
			pipelineLayoutInfo.pSetLayouts = layouts.data();
			m_ComputePipelineLayoutClouds = device->GetDevice().createPipelineLayout(pipelineLayoutInfo, nullptr);


			vk::ComputePipelineCreateInfo pipelineInfo{};

			pipelineInfo.sType = vk::StructureType::eComputePipelineCreateInfo;
			pipelineInfo.layout = m_ComputePipelineLayoutClouds;
			pipelineInfo.stage = computeShaderStageInfo;
			m_ComputePipelineClouds = device->GetDevice().createComputePipeline(nullptr, pipelineInfo).value;



			vkDestroyShaderModule(device->GetDevice(), computeShaderModule, nullptr);
		}

	}

	void Renderer::UpdateCloudTexture()
	{
		auto device = m_Device->GetDevice();
		if (m_3DNoiseTexture->m_ImageLayout != vk::ImageLayout::eGeneral)
		{
			auto cmdBuffer = m_ComputeCommandBuffer[currentFrame].BeginTransfering();


			m_ComputeCommandBuffer[currentFrame].ChangeImageLayout(m_3DNoiseTexture.get(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral);
			m_ComputeCommandBuffer[currentFrame].ChangeImageLayout(m_3DNoiseTextureLowRes.get(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral);
			m_ComputeCommandBuffer[currentFrame].EndTransfering();
			m_ComputeCommandBuffer[currentFrame].SubmitSingle();

		}
		auto cmdBuffer = m_ComputeCommandBuffer[currentFrame].BeginTransfering();




		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipelineClouds);
		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_ComputePipelineLayoutClouds, 0, 1, &m_DescriptorSets[currentFrame], 0, 0);
		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_ComputePipelineLayoutClouds, 1, 1, &m_DescriptorSetNoise, 0, 0);
		float invocations = 128;
		int localSize= 8;
	

		vkCmdDispatch(cmdBuffer, invocations/ localSize, invocations/ localSize, invocations/ localSize);

		m_ComputeCommandBuffer[currentFrame].ChangeImageLayout(m_3DNoiseTexture.get(), vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal);
		m_ComputeCommandBuffer[currentFrame].ChangeImageLayout(m_3DNoiseTextureLowRes.get(), vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal);
		m_ComputeCommandBuffer[currentFrame].EndTransfering();
		m_ComputeCommandBuffer[currentFrame].SubmitSingle();


		device.waitIdle();
	}


	void Renderer::UpdateNoiseTexture()
	{
		auto device = m_Device->GetDevice();
		if (m_NoiseImage->m_ImageLayout != vk::ImageLayout::eGeneral)
		{
			auto cmdBuffer = m_ComputeCommandBuffer[currentFrame].BeginTransfering();
			
			
			m_ComputeCommandBuffer[currentFrame].ChangeImageLayout(m_NoiseImage.get(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral);
			m_ComputeCommandBuffer[currentFrame].ChangeImageLayout(m_AnimatedNoiseImage.get(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral);
			
			m_ComputeCommandBuffer[currentFrame].EndTransfering();
			m_ComputeCommandBuffer[currentFrame].SubmitSingle();

		}
		auto cmdBuffer = m_ComputeCommandBuffer[currentFrame].BeginTransfering();




		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline);
		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_ComputePipelineLayout, 0, 1, &m_DescriptorSets[currentFrame], 0, 0);
		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_ComputePipelineLayout, 1, 1, &m_DescriptorSetNoise, 0, 0);

		vkCmdDispatch(cmdBuffer, noiseTextureWidth / 16, noiseTextureHeight / 16, 1);

		m_ComputeCommandBuffer[currentFrame].ChangeImageLayout(m_NoiseImage.get(), vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal);
		m_ComputeCommandBuffer[currentFrame].ChangeImageLayout(m_AnimatedNoiseImage.get(), vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal);
		m_ComputeCommandBuffer[currentFrame].EndTransfering();
		m_ComputeCommandBuffer[currentFrame].SubmitSingle();


		device.waitIdle();

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

		vk::DescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = 0;
		layoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
		layoutBinding.stageFlags = vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eVertex| vk::ShaderStageFlagBits::eTessellationControl
			| vk::ShaderStageFlagBits::eTessellationEvaluation | vk::ShaderStageFlagBits::eFragment;
		layoutBinding.descriptorCount = 1;

		vk::DescriptorSetLayoutBinding layoutBinding1;
		layoutBinding1.binding = 1;
		layoutBinding1.descriptorType = vk::DescriptorType::eInputAttachment;
		layoutBinding1.stageFlags = vk::ShaderStageFlagBits::eFragment;
		layoutBinding1.descriptorCount = 1;
		std::vector<vk::DescriptorSetLayoutBinding> layoutBindings{ layoutBinding,layoutBinding1 };
		m_DescriptorSetLayout = DescriptorSetLayout::Create(layoutBindings);

		{

			vk::DescriptorSetLayoutBinding layoutBinding;
			layoutBinding.binding = 0;
			layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			layoutBinding.stageFlags =  vk::ShaderStageFlagBits::eFragment;
			layoutBinding.descriptorCount = 1;

		
			std::vector<vk::DescriptorSetLayoutBinding> layoutBindings1{ layoutBinding };
			m_DescriptorSetLayoutSky = DescriptorSetLayout::Create(layoutBindings1);

			vk::DescriptorPoolSize poolSize;
			poolSize.type = vk::DescriptorType::eCombinedImageSampler;
			poolSize.descriptorCount = 1;

			std::vector<vk::DescriptorPoolSize> poolSizes{ poolSize };

			m_DescriptorPoolSky = DescriptorPool::Create(poolSizes, 1);
			std::vector<vk::DescriptorSetLayout> layouts = { m_DescriptorSetLayoutSky->GetLayout() };
			m_DescriptorSetSky = m_DescriptorPoolSky->AllocateDescriptorSets(1, layouts.data())[0];
		}
		// clouds
		{
			vk::DescriptorSetLayoutBinding layoutBinding;
			layoutBinding.binding = 0;
			layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			layoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
			layoutBinding.descriptorCount = 1;
			vk::DescriptorSetLayoutBinding layoutBinding1;
			layoutBinding1.binding = 1;
			layoutBinding1.descriptorType = vk::DescriptorType::eUniformBuffer;
			layoutBinding1.stageFlags = vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex;
			layoutBinding1.descriptorCount = 1;

			vk::DescriptorSetLayoutBinding layoutBinding2;
			layoutBinding2.binding = 2;
			layoutBinding2.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			layoutBinding2.stageFlags = vk::ShaderStageFlagBits::eFragment;
			layoutBinding2.descriptorCount = 1;

			std::vector<vk::DescriptorSetLayoutBinding> layoutBindings1{ layoutBinding,layoutBinding1,layoutBinding2 };
			m_DescriptorSetLayoutClouds = DescriptorSetLayout::Create(layoutBindings1);



			vk::DescriptorPoolSize poolSize;
			poolSize.type = vk::DescriptorType::eCombinedImageSampler;
			poolSize.descriptorCount = 1;
			std::vector<vk::DescriptorPoolSize> poolSizes{ poolSize};

			m_DescriptorPoolClouds = DescriptorPool::Create(poolSizes, 1);
			m_DescriptorSetClouds = m_DescriptorPoolClouds->AllocateDescriptorSets(1,&m_DescriptorSetLayoutClouds->GetLayout())[0];


			BufferInputChunk inputBuffer;
			inputBuffer.size = sizeof(CloudParams);
			inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			inputBuffer.usage = vk::BufferUsageFlagBits::eUniformBuffer;
			m_CloudBuffer = CreateUPtr<Buffer>(inputBuffer);
			m_CloudPtr = m_Device->GetDevice().mapMemory(m_CloudBuffer->GetMemory(), 0, sizeof(CloudParams));
			memcpy(m_CloudPtr, &cloudParams, sizeof(CloudParams));
			m_Device->UpdateDescriptorSet(m_DescriptorSetClouds, 1, 1, *m_CloudBuffer, vk::DescriptorType::eUniformBuffer);
		}


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
			poolSize1.type = vk::DescriptorType::eInputAttachment;
			poolSize1.descriptorCount = static_cast<uint32_t>(framesAmount);

			std::vector<vk::DescriptorPoolSize> poolSizes{ poolSize,poolSize1 };
			
			m_DescriptorPool = DescriptorPool::Create(poolSizes, framesAmount);
		}
		

		std::vector<vk::DescriptorSetLayout> layouts(framesAmount, m_DescriptorSetLayout->GetLayout());

		
		m_DescriptorSets =  m_DescriptorPool->AllocateDescriptorSets(framesAmount, layouts.data());
			

		for (int i = 0; i < framesAmount; i++)
		{
		
			
			m_Device->UpdateDescriptorSet(m_DescriptorSets[i],0,1, *m_UniformBuffers[i], vk::DescriptorType::eUniformBuffer);
			vk::DescriptorImageInfo imageInfo;
			imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			imageInfo.imageView = m_Swapchain->m_SwapchainFrames[i].imageDepthView;
			imageInfo.sampler = VK_NULL_HANDLE; 
			m_Device->UpdateDescriptorSet(m_DescriptorSets[i], 1, 1, imageInfo, vk::DescriptorType::eInputAttachment);

		}
		

		{

			vk::DescriptorPoolSize poolSize;
			poolSize.type = vk::DescriptorType::eCombinedImageSampler;
			poolSize.descriptorCount = 6;
		
			std::vector<vk::DescriptorPoolSize> poolSizes{poolSize};

			m_DescriptorPoolTex = DescriptorPool::Create(poolSizes,1);

			std::vector<vk::DescriptorSetLayout> layouts;
			layouts.resize(1);
			
		

			vk::DescriptorSetLayoutBinding layoutBinding;
			layoutBinding.binding = 0;
			layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			layoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eTessellationEvaluation | vk::ShaderStageFlagBits::eFragment;
			layoutBinding.descriptorCount = 2;


			vk::DescriptorSetLayoutBinding layoutBinding1;
			layoutBinding1.binding = 1;
			layoutBinding1.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			layoutBinding1.stageFlags = vk::ShaderStageFlagBits::eFragment;
			layoutBinding1.descriptorCount = 1;

			vk::DescriptorSetLayoutBinding layoutBinding2;
			layoutBinding2.binding = 2;
			layoutBinding2.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			layoutBinding2.stageFlags = vk::ShaderStageFlagBits::eFragment;
			layoutBinding2.descriptorCount = 1;

			vk::DescriptorSetLayoutBinding layoutBinding3;
			layoutBinding3.binding = 3;
			layoutBinding3.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			layoutBinding3.stageFlags = vk::ShaderStageFlagBits::eFragment;
			layoutBinding3.descriptorCount = 1;

			vk::DescriptorSetLayoutBinding layoutBinding4;
			layoutBinding4.binding = 4;
			layoutBinding4.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			layoutBinding4.stageFlags = vk::ShaderStageFlagBits::eFragment;
			layoutBinding4.descriptorCount = 1;

			vk::DescriptorSetLayoutBinding layoutBinding5;
			layoutBinding5.binding = 5;
			layoutBinding5.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			layoutBinding5.stageFlags = vk::ShaderStageFlagBits::eFragment;
			layoutBinding5.descriptorCount = 1;

			std::vector<vk::DescriptorSetLayoutBinding> layoutBindings{ layoutBinding,layoutBinding1,layoutBinding2,layoutBinding3,layoutBinding4,layoutBinding5 };

			m_DescriptorSetLayoutTex = DescriptorSetLayout::Create(layoutBindings);

			layouts[0] = m_DescriptorSetLayoutTex->GetLayout();
			m_DescriptorSetTex = m_DescriptorPoolTex->AllocateDescriptorSets(1, layouts.data())[0];
		}




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
		
		{
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


			const std::vector<uint32_t> indices =
			{
				0, 1, 3, 3, 1, 2,
				1, 5, 2, 2, 5, 6,
				5, 4, 6, 6, 4, 7,
				4, 0, 7, 7, 0, 3,
				3, 2, 7, 7, 2, 6,
				4, 5, 0, 0, 5, 1
			};
			{
				auto indexSize = sizeof(indices[0]) * indices.size();
				SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(indexSize);


				{
					BufferInputChunk inputBuffer;
					inputBuffer.size = indexSize;
					inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
					inputBuffer.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
					m_IndexCubeBuffer = CreateUPtr<IndexBuffer>(inputBuffer, indices.size(), vk::IndexType::eUint32);

				}

				m_TransferCommandBuffer[0].BeginTransfering();
				m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_IndexCubeBuffer.get(), (void*)indices.data(), indexSize);
				m_TransferCommandBuffer[0].EndTransfering();
				m_TransferCommandBuffer[0].SubmitSingle();



			}

			auto vertexSize = SizeOfBuffer(CubeVerticies.size(), CubeVerticies[0]);
			void* vertexData = const_cast<void*>(static_cast<const void*>(CubeVerticies.data()));
			SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(vertexSize);
			{
				BufferInputChunk inputBuffer;
				inputBuffer.size = vertexSize;
				inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
				inputBuffer.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;

				m_CubeBuffer = CreateUPtr<Buffer>(inputBuffer);
			}

			m_TransferCommandBuffer[0].BeginTransfering();
			m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_CubeBuffer.get(), (void*)CubeVerticies.data(), vertexSize);
			m_TransferCommandBuffer[0].EndTransfering();
			m_TransferCommandBuffer[0].SubmitSingle();

		}
		

		CreateMSAAFrame();

	
		

		CreateComputePipeline();
		CreatePipeline();
		CreateFramebuffers();

		auto GetRandomNumber = []() {
			std::random_device rd;
			std::default_random_engine e{ rd() };
			std::uniform_real_distribution<float> dist{ 0.0f, 1.0f };
			return dist(e);
		};

		
		// create a buffer with points
		int numCellsPerAxis = noiseData.cellAmountA;
		float cellSize = 1.f/ numCellsPerAxis;

		std::vector<glm::vec3> points;
#if 1
		points.resize(numCellsPerAxis* numCellsPerAxis* numCellsPerAxis);
		for (int x = 0; x < numCellsPerAxis; x++) {
			for (int y = 0; y < numCellsPerAxis; y++) {
				for (int z = 0; z < numCellsPerAxis; z++) {
					float randomX = GetRandomNumber();
					float randomY = GetRandomNumber();
					float randomZ = GetRandomNumber();
					glm::vec3 randomOffset = glm::vec3(randomX, randomY, randomZ);
					glm::vec3 cellCorner = glm::vec3(x, y, z);
					glm::vec3 pos = (cellCorner + randomOffset) * cellSize;
					int index = x + numCellsPerAxis * (y + z * numCellsPerAxis);
					assert(index < points.size());
					points[index] = pos;
				}
			}
		}
#else
		points.resize(numCellsPerAxis * numCellsPerAxis);
		for (int x = 0; x < numCellsPerAxis; x++) {
			for (int y = 0; y < numCellsPerAxis; y++) {
					float randomX = GetRandomNumber();
					float randomY = GetRandomNumber();
					float randomZ = GetRandomNumber();
					glm::vec3 randomOffset = glm::vec3(randomX, randomY, 0) * cellSize;
					glm::vec3 cellCorner = glm::vec3(x, y, 0) * cellSize;

					int index = x + numCellsPerAxis * (y);
					assert(index < points.size());
					points[index] = cellCorner + randomOffset;
			}
		}
#endif // 0

		

		/*{
			auto bufferSize = points.size() * sizeof(glm::vec3);
			{
				SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(bufferSize);


				{
					BufferInputChunk inputBuffer;
					inputBuffer.size = bufferSize;
					inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
					inputBuffer.usage = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst;
					m_PointsData = CreateUPtr<Buffer>(inputBuffer);

				}

				m_TransferCommandBuffer[0].BeginTransfering();
				m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_PointsData.get(), (void*)points.data(), bufferSize);
				m_TransferCommandBuffer[0].EndTransfering();
				m_TransferCommandBuffer[0].SubmitSingle();


				m_Device->UpdateDescriptorSet(m_DescriptorSetNoise, 4, 1, *m_PointsData, vk::DescriptorType::eStorageBuffer);
			}
		}*/

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


		UpdateNoiseTexture();
		UpdateCloudTexture();
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


	void Renderer::Render(float deltaTime)
	{
		exeTime += deltaTime;
		m_InstanceData.clear();
		auto cameraPos = m_App->GetCamera()->m_Position;
		//cameraPos = { 0,0,0 };
		auto quadTree = Quadtree::Build(cameraPos);
		
		auto test1 = glm::vec3(-0.5, 0, -0.5) * 9.705f + glm::vec3(-4.8828, 0, 24.414);
		glm::vec4 test = m_App->GetCamera()->GetProj() * m_App->GetCamera()->GetView() * glm::vec4(test1,1);
		auto norm = test / test.w;
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
		auto swapchain = m_Swapchain->m_Swapchain;
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

		
		m_TransferCommandBuffer[imageIndex].EndTransfering();
		m_TransferCommandBuffer[imageIndex].SubmitSingle();
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



				TracyVkZone(ctx, commandBuffer, "Terrain Rendering ");

#if 0
				// z prepass
				{
					m_RenderCommandBuffer[imageIndex].BeginRenderPass(&m_ZTerrainPipeline->m_RenderPass, &m_Swapchain->m_SwapchainFrames[imageIndex].framebuffer, &m_Swapchain->m_SwapchainExtent);
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

					commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_ZTerrainPipeline->m_PipelineLayout, 0, m_DescriptorSets[imageIndex], nullptr);
					commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_ZTerrainPipeline->m_PipelineLayout, 1, m_DescriptorSetTex, nullptr);
					commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_ZTerrainPipeline->m_PipelineLayout, 2, m_DescriptorSetNoise, nullptr);

					commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_ZTerrainPipeline->m_Pipeline);
					vk::DeviceSize offsets[] = { 0 };

					{
						vk::Buffer vertexBuffers[] = { m_ModelBuffer->GetBuffer() };
						commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

					}

					{
						vk::Buffer vertexBuffers[] = { m_InstancedDataBuffer->GetBuffer() };
						commandBuffer.bindVertexBuffers(1, 1, vertexBuffers, offsets);

					}


					commandBuffer.setViewport(0, 1, &viewport);
					commandBuffer.setScissor(0, 1, &scissors);

					commandBuffer.bindIndexBuffer(m_IndexBuffer->GetBuffer(), 0, m_IndexBuffer->GetIndexType());
					commandBuffer.drawIndexed(static_cast<uint32_t>(amount), m_InstanceData.size(), 0, 0, 0);


					vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
					// water rendering
					{

						commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_ZWaterPipeline->m_Pipeline);
						commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_ZWaterPipeline->m_PipelineLayout, 0, m_DescriptorSets[imageIndex], nullptr);
						commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_ZWaterPipeline->m_PipelineLayout, 1, m_DescriptorSetTex, nullptr);
						commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_ZWaterPipeline->m_PipelineLayout, 2, m_DescriptorSetNoise, nullptr);
						commandBuffer.drawIndexed(static_cast<uint32_t>(amount), m_InstanceData.size(), 0, 0, 0);
					}
				}

				m_Device->GetDevice().waitIdle();
#endif // 0

				
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


				//sky
				{
					commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_SkyPipeline->m_PipelineLayout, 0, m_DescriptorSets[imageIndex], nullptr);
					commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_SkyPipeline->m_PipelineLayout, 1, m_DescriptorSetSky, nullptr);
				
					commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_SkyPipeline->m_Pipeline);
				
					commandBuffer.setViewport(0, 1, &viewport);
					commandBuffer.setScissor(0, 1, &scissors);
				
					commandBuffer.draw(6, 1, 0, 0);
				}


				

				vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
				commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_TerrainPipeline->m_PipelineLayout, 0, m_DescriptorSets[imageIndex], nullptr);
				commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_TerrainPipeline->m_PipelineLayout, 1, m_DescriptorSetTex, nullptr);
				commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_TerrainPipeline->m_PipelineLayout, 2, m_DescriptorSetNoise, nullptr);

				commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_TerrainPipeline->m_Pipeline);
				vk::DeviceSize offsets[] = { 0 };

				{
					vk::Buffer vertexBuffers[] = { m_ModelBuffer->GetBuffer() };
					commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

				}

				{
					vk::Buffer vertexBuffers[] = { m_InstancedDataBuffer->GetBuffer() };
					commandBuffer.bindVertexBuffers(1, 1, vertexBuffers, offsets);

				}


				commandBuffer.setViewport(0, 1, &viewport);
				commandBuffer.setScissor(0, 1, &scissors);

				commandBuffer.bindIndexBuffer(m_IndexBuffer->GetBuffer(), 0, m_IndexBuffer->GetIndexType());
				commandBuffer.drawIndexed(static_cast<uint32_t>(amount), m_InstanceData.size(), 0, 0, 0);

			
			vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
			// water rendering
				{

				commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_WaterPipeline->m_Pipeline);
				commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_WaterPipeline->m_PipelineLayout, 0, m_DescriptorSets[imageIndex], nullptr);
				commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_WaterPipeline->m_PipelineLayout, 1, m_DescriptorSetTex, nullptr);
				commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_WaterPipeline->m_PipelineLayout, 2, m_DescriptorSetNoise, nullptr);
				commandBuffer.drawIndexed(static_cast<uint32_t>(amount), m_InstanceData.size(), 0, 0, 0);
				}


				vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
			// ray march rendering
				{

					commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_RayMarchPipeline->m_Pipeline);
					commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_RayMarchPipeline->m_PipelineLayout, 0, m_DescriptorSets[imageIndex], nullptr);
					commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_RayMarchPipeline->m_PipelineLayout, 1, m_DescriptorSetClouds, nullptr);
					{
						vk::Buffer vertexBuffers[] = { m_CubeBuffer->GetBuffer() };
						//commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

					}
					//commandBuffer.bindIndexBuffer(m_IndexCubeBuffer->GetBuffer(), 0, m_IndexCubeBuffer->GetIndexType());
					//commandBuffer.drawIndexed(m_IndexCubeBuffer->GetIndexAmount(), 1, 0, 0, 0);
					commandBuffer.draw(6, 1, 0, 0);
				}
			}
			m_RenderCommandBuffer[imageIndex].EndRenderPass();
			TracyVkCollect(ctx, commandBuffer);
			commandBuffer.end();
			renderCommandBuffer.EndRendering();

		}


		

		
			
#if IMGUI_ENABLED
		
			RenderImGui(imageIndex);

#endif

			vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
			vk::SubmitInfo submitInfo = {};

			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			const std::vector<vk::CommandBuffer> commandBuffers = { renderCommandBuffer.GetCommandBuffer(), g_MainWindowData.Frames[imageIndex].CommandBuffer };
			//const std::vector<vk::CommandBuffer> commandBuffers = { renderCommandBuffer.GetCommandBuffer() };

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



		if (m_IsResized)
		{
			m_Device->GetDevice().waitIdle();
			m_NoiseImage.reset();
			m_NoiseImage = Image::CreateEmptyImage(noiseTextureWidth, noiseTextureHeight,vk::Format::eR8G8B8A8Snorm);
			m_Device->UpdateDescriptorSet(m_DescriptorSetNoise, 0, 1, *m_NoiseImage, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage);
			m_Device->UpdateDescriptorSet(m_DescriptorSetTex, 0, 1, *m_NoiseImage, vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eCombinedImageSampler);
		}
		// clouds terrain and water be updated separately
		if (true)
		{
			// update noise descriptor 
			//m_IsNewParametrs = false;
			m_Device->GetDevice().waitIdle();
			memcpy(m_NoiseDataPtr, &noiseData, sizeof(NoiseData));
			UpdateNoiseTexture();
			UpdateCloudTexture();
		}
		if (m_IsNewParametrs)
		{
			m_IsNewParametrs = false;
			memcpy(m_CloudPtr, &cloudParams, sizeof(CloudParams));
		}
		if (m_IsPolygon)
		{
			m_Device->GetDevice().waitIdle();
			m_IsPolygon = false;
			if (m_PolygoneMode == vk::PolygonMode::eLine)
			{
				m_PolygoneMode = vk::PolygonMode::eFill;

			}
			else
			{
				m_PolygoneMode = vk::PolygonMode::eLine;

			}
			m_TerrainPipeline.reset();
			m_WaterPipeline.reset();
			CreatePipeline();
			
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


	static vk::RenderPass MakeRenderPass(vk::Device device, vk::Format swapchainImageFormat, vk::Format depthFormat)
	{

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

		vk::SubpassDescription subpass0 = {};
		subpass0.flags = vk::SubpassDescriptionFlags();
		subpass0.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass0.colorAttachmentCount = 1;
		subpass0.pColorAttachments = &msaaAttachmentRef;

		vk::SubpassDescription subpass = {};
		subpass.flags = vk::SubpassDescriptionFlags();
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &msaaAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		//subpass.pResolveAttachments = &colorAttachmentRef;



		vk::AttachmentReference depthStencilAttachmentRef = {};
		depthStencilAttachmentRef.attachment = 1; // The index of the depth/stencil attachment in the attachments array
		depthStencilAttachmentRef.layout = vk::ImageLayout::eGeneral; // The layout of the depth/stencil attachment

		vk::SubpassDescription subpass1 = {};
		subpass1.flags = vk::SubpassDescriptionFlags();
		subpass1.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass1.colorAttachmentCount = 1;
		subpass1.pColorAttachments = &msaaAttachmentRef;

		vk::SubpassDescription subpass2 = {};
		subpass2.flags = vk::SubpassDescriptionFlags();
		subpass2.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass2.colorAttachmentCount = 1;
		subpass2.pColorAttachments = &msaaAttachmentRef;
		subpass2.pResolveAttachments = &colorAttachmentRef;
		//subpass2.pDepthStencilAttachment = &depthStencilAttachmentRef;

		std::vector<vk::AttachmentReference>inputReferences;
		{
			vk::AttachmentReference reference = {};
			reference.attachment = 1; // The index of the input attachment in the attachments array
			reference.layout = vk::ImageLayout::eShaderReadOnlyOptimal; // The layout of the input attachment
			inputReferences .push_back(reference );
			subpass1.inputAttachmentCount = inputReferences.size();
			subpass1.pInputAttachments = inputReferences.data();
			subpass2.inputAttachmentCount = inputReferences.size();
			subpass2.pInputAttachments = inputReferences.data();

		}
		
		std::vector<vk::SubpassDescription> subpasses = { subpass0 , subpass ,subpass1,subpass2 };


		vk::SubpassDependency dependency0{};
		dependency0.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency0.dstSubpass = 0;
		dependency0.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency0.srcAccessMask = vk::AccessFlagBits::eNone;
		dependency0.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency0.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

		vk::SubpassDependency dependency1 = {};
		dependency1.srcSubpass = 0;
		dependency1.dstSubpass = 1;
		dependency1.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency1.srcAccessMask = vk::AccessFlagBits::eNone;
		dependency1.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency1.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

		vk::SubpassDependency dependency2 = {};
		dependency2.srcSubpass = 1;
		dependency2.dstSubpass = 2;
		dependency2.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency2.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		dependency2.dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
		dependency2.dstAccessMask = vk::AccessFlagBits::eInputAttachmentRead;

		vk::SubpassDependency dependency3 = {};
		dependency3.srcSubpass = 2;
		dependency3.dstSubpass = 3;
		dependency3.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency3.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		dependency3.dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
		dependency3.dstAccessMask = vk::AccessFlagBits::eInputAttachmentRead;

		std::vector<vk::SubpassDependency> dependencies = { dependency0 ,dependency1,dependency2,dependency3 };
		//// define order of subpasses?
		//// where srcAccess mask operations occurr
		////list operatoins that must be completed before staring render pass
		//dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		////dependency.srcAccessMask = vk::AccessFlagBits::eNone;
		//dependency.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		//// define rights of subpasses?
		//dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		//dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

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
		auto swapchainFormat = m_Swapchain->m_SwapchainFormat;
		auto swapChainExtent = m_Swapchain->m_SwapchainExtent;


		


		// terrain pipeline
		{
			GraphicsPipelineSpecification specs;

			specs.device = m_Device->GetDevice();

			specs.vertexFilepath = BASE_SPIRV_OUTPUT + "noise.spvV";
			specs.fragmentFilepath = BASE_SPIRV_OUTPUT + "default.spvF";
			specs.tessCFilepath = BASE_SPIRV_OUTPUT + "tess.spvC";
			specs.tessEFilepath = BASE_SPIRV_OUTPUT + "tess.spvE";
			specs.swapchainExtent = swapChainExtent;
			specs.swapchainImageFormat = swapchainFormat;


			std::vector<vk::VertexInputBindingDescription> bindings{ CreateBindingDescription(0,sizeof(Vertex),vk::VertexInputRate::eVertex) ,CreateBindingDescription(1,sizeof(InstanceData),vk::VertexInputRate::eInstance) };

			std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;

			attributeDescriptions =
			{
				VertexInputAttributeDescription(0,0,vk::Format::eR32G32B32Sfloat,offsetof(Vertex, Position)),
				VertexInputAttributeDescription(0,1,vk::Format::eR32G32B32A32Sfloat,offsetof(Vertex, Color)),
				VertexInputAttributeDescription(0,2,vk::Format::eR32G32Sfloat,offsetof(Vertex, UV)),

				VertexInputAttributeDescription(1,4,vk::Format::eR32G32B32Sfloat,offsetof(InstanceData, pos)),
				VertexInputAttributeDescription(1,5,vk::Format::eR32G32B32A32Sfloat,offsetof(InstanceData, edges)),
				VertexInputAttributeDescription(1,6,vk::Format::eR32Sfloat,offsetof(InstanceData, scale)),
			};

			specs.bindingDescription = bindings;



			specs.attributeDescription = attributeDescriptions;
			//specs.attributeDescription = Particle::GetAttributeDescriptions();

			auto samples = RenderContext::GetDevice()->GetSamples();
			specs.samples = samples;
			auto pipelineLayouts = std::vector<vk::DescriptorSetLayout>{ m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutTex->GetLayout(),m_DescriptorSetLayoutNoise->GetLayout() };

			specs.descriptorSetLayout = pipelineLayouts;

			#if 0
			// z render pass creation
			{
				auto device = m_Device->GetDevice();
				auto samples = RenderContext::GetDevice()->GetSamples();
				auto swapchainImageFormat = specs.swapchainImageFormat;
				auto depthFormat = m_Swapchain->m_SwapchainFrames[0].depthFormat;



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
				depthAttachmentRef.attachment = 0;
				depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;



				//Renderpasses are broken down into subpasses, there's always at least one.
				vk::SubpassDescription subpass = {};
				subpass.flags = vk::SubpassDescriptionFlags();
				subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
				subpass.pDepthStencilAttachment = &depthAttachmentRef;





				std::vector<vk::SubpassDescription> subpasses = { subpass };


				

					//Now create the renderpass
				vk::RenderPassCreateInfo renderpassInfo = {};

				std::vector<vk::AttachmentDescription> attachments = { depthAttachment };
				renderpassInfo.flags = vk::RenderPassCreateFlags();
				renderpassInfo.attachmentCount = attachments.size();
				renderpassInfo.pAttachments = attachments.data();
				renderpassInfo.subpassCount = subpasses.size();
				renderpassInfo.pSubpasses = subpasses.data();
			

				try {
					m_ZPass = device.createRenderPass(renderpassInfo);
				}
				catch (vk::SystemError err)
				{
					Log::GetLog()->error("Failed to create Z prepass!");
				}

				// create Z pipelines

				{

					specs.vertexFilepath = BASE_SPIRV_OUTPUT + "noise.spvV";
					specs.fragmentFilepath = BASE_SPIRV_OUTPUT + "ZPrepass.spvF";
					specs.tessCFilepath = BASE_SPIRV_OUTPUT + "tess.spvC";
					specs.tessEFilepath = BASE_SPIRV_OUTPUT + "tess.spvE";
					attributeDescriptions =
					{
						VertexInputAttributeDescription(0,0,vk::Format::eR32G32B32Sfloat,offsetof(Vertex, Position)),
						VertexInputAttributeDescription(0,1,vk::Format::eR32G32B32A32Sfloat,offsetof(Vertex, Color)),
						VertexInputAttributeDescription(0,2,vk::Format::eR32G32Sfloat,offsetof(Vertex, UV)),

						VertexInputAttributeDescription(1,4,vk::Format::eR32G32B32Sfloat,offsetof(InstanceData, pos)),
						VertexInputAttributeDescription(1,5,vk::Format::eR32G32B32A32Sfloat,offsetof(InstanceData, edges)),
						VertexInputAttributeDescription(1,6,vk::Format::eR32Sfloat,offsetof(InstanceData, scale)),
					};

					specs.bindingDescription = bindings;
					m_ZTerrainPipeline = Pipeline::CreateGraphicsPipeline(specs, vk::PrimitiveTopology::ePatchList, m_Swapchain->m_SwapchainFrames[0].depthFormat, m_ZPass, 0, true, m_PolygoneMode);
				}
				{

					specs.vertexFilepath = BASE_SPIRV_OUTPUT + "water.spvV";
					specs.fragmentFilepath = BASE_SPIRV_OUTPUT + "ZPrepass.spvF";
					specs.tessCFilepath = BASE_SPIRV_OUTPUT + "waterTess.spvC";
					specs.tessEFilepath = BASE_SPIRV_OUTPUT + "waterTess.spvE";;
					attributeDescriptions =
					{
						VertexInputAttributeDescription(0,0,vk::Format::eR32G32B32Sfloat,offsetof(Vertex, Position)),
						VertexInputAttributeDescription(0,1,vk::Format::eR32G32B32A32Sfloat,offsetof(Vertex, Color)),
						VertexInputAttributeDescription(0,2,vk::Format::eR32G32Sfloat,offsetof(Vertex, UV)),

						VertexInputAttributeDescription(1,4,vk::Format::eR32G32B32Sfloat,offsetof(InstanceData, pos)),
						VertexInputAttributeDescription(1,5,vk::Format::eR32G32B32A32Sfloat,offsetof(InstanceData, edges)),
						VertexInputAttributeDescription(1,6,vk::Format::eR32Sfloat,offsetof(InstanceData, scale)),
					};

					specs.bindingDescription = bindings;
				m_ZWaterPipeline = Pipeline::CreateGraphicsPipeline(specs, vk::PrimitiveTopology::ePatchList, m_Swapchain->m_SwapchainFrames[0].depthFormat, m_ZPass, 0, true, m_PolygoneMode);
				}
			}
			#endif // 0

			



			auto terrainRenderPass = MakeRenderPass(m_Device->GetDevice(), specs.swapchainImageFormat,m_Swapchain->m_SwapchainFrames[0].depthFormat);
			m_TerrainPipeline = Pipeline::CreateGraphicsPipeline(specs, vk::PrimitiveTopology::ePatchList, m_Swapchain->m_SwapchainFrames[0].depthFormat, terrainRenderPass,1,true,m_PolygoneMode);
		
		}
		
		// water pipeline 
		{
			GraphicsPipelineSpecification specs;

			
			
			specs.device = m_Device->GetDevice();

			specs.vertexFilepath = BASE_SPIRV_OUTPUT + "water.spvV";
			specs.fragmentFilepath = BASE_SPIRV_OUTPUT + "water.spvF";
			specs.tessCFilepath = BASE_SPIRV_OUTPUT + "waterTess.spvC";
			specs.tessEFilepath = BASE_SPIRV_OUTPUT + "waterTess.spvE";
			specs.swapchainExtent = swapChainExtent;
			specs.swapchainImageFormat = swapchainFormat;


			std::vector<vk::VertexInputBindingDescription> bindings{ CreateBindingDescription(0,sizeof(Vertex),vk::VertexInputRate::eVertex) ,CreateBindingDescription(1,sizeof(InstanceData),vk::VertexInputRate::eInstance) };

			std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;

			attributeDescriptions =
			{
				VertexInputAttributeDescription(0,0,vk::Format::eR32G32B32Sfloat,offsetof(Vertex, Position)),
				VertexInputAttributeDescription(0,1,vk::Format::eR32G32B32A32Sfloat,offsetof(Vertex, Color)),
				VertexInputAttributeDescription(0,2,vk::Format::eR32G32Sfloat,offsetof(Vertex, UV)),

				VertexInputAttributeDescription(1,4,vk::Format::eR32G32B32Sfloat,offsetof(InstanceData, pos)),
				VertexInputAttributeDescription(1,5,vk::Format::eR32G32B32A32Sfloat,offsetof(InstanceData, edges)),
				VertexInputAttributeDescription(1,6,vk::Format::eR32Sfloat,offsetof(InstanceData, scale)),
			};

			specs.bindingDescription = bindings;



			specs.attributeDescription = attributeDescriptions;

			auto samples = RenderContext::GetDevice()->GetSamples();
			specs.samples = samples;




			auto pipelineLayouts = std::vector<vk::DescriptorSetLayout>{ m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutTex->GetLayout(),m_DescriptorSetLayoutNoise->GetLayout()};

			specs.descriptorSetLayout = pipelineLayouts;
		

			m_WaterPipeline = Pipeline::CreateGraphicsPipeline(specs, vk::PrimitiveTopology::ePatchList, m_Swapchain->m_SwapchainFrames[0].depthFormat, m_TerrainPipeline->m_RenderPass,2,false,m_PolygoneMode);
		}
		// sky pipeline 
		{

			GraphicsPipelineSpecification specs;



			specs.device = m_Device->GetDevice();

			specs.vertexFilepath = BASE_SPIRV_OUTPUT + "sky.spvV";
			specs.fragmentFilepath = BASE_SPIRV_OUTPUT + "sky.spvF";
			specs.swapchainExtent = swapChainExtent;
			specs.swapchainImageFormat = swapchainFormat;








			auto samples = RenderContext::GetDevice()->GetSamples();
			specs.samples = samples;




			auto pipelineLayouts = std::vector<vk::DescriptorSetLayout>{ m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutSky->GetLayout()};

			specs.descriptorSetLayout = pipelineLayouts;


			m_SkyPipeline = Pipeline::CreateGraphicsPipeline(specs, vk::PrimitiveTopology::eTriangleList, m_Swapchain->m_SwapchainFrames[0].depthFormat, m_WaterPipeline->m_RenderPass, 0, false, m_PolygoneMode);
		}
		// ray march
		// sky pipeline 
		{

			GraphicsPipelineSpecification specs;



			specs.device = m_Device->GetDevice();

			specs.vertexFilepath = BASE_SPIRV_OUTPUT + "rayMarch.spvV";
			specs.fragmentFilepath = BASE_SPIRV_OUTPUT + "rayMarch.spvF";
			specs.swapchainExtent = swapChainExtent;
			specs.swapchainImageFormat = swapchainFormat;








			auto samples = RenderContext::GetDevice()->GetSamples();
			specs.samples = samples;

			auto pipelineLayouts = std::vector<vk::DescriptorSetLayout>{ m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutClouds->GetLayout() };
			std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;

			attributeDescriptions =
			{
				VertexInputAttributeDescription(0,0,vk::Format::eR32G32B32Sfloat,offsetof(Vertex, Position)),
				VertexInputAttributeDescription(0,1,vk::Format::eR32G32Sfloat,offsetof(Vertex, UV))
			};
			std::vector<vk::VertexInputBindingDescription> bindings{ CreateBindingDescription(0,sizeof(Vertex),vk::VertexInputRate::eVertex) };
			specs.descriptorSetLayout = pipelineLayouts;
			//specs.attributeDescription = attributeDescriptions;
			//specs.bindingDescription =bindings;





			m_RayMarchPipeline = Pipeline::CreateGraphicsPipeline(specs, vk::PrimitiveTopology::eTriangleList, m_Swapchain->m_SwapchainFrames[0].depthFormat, m_WaterPipeline->m_RenderPass, 3, true, m_PolygoneMode);
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



		ImGui::Begin("NoiseParametrs", &show_another_window);
		ImGui::Text("Terrain");
		m_IsNewParametrs |= ImGui::SliderFloat("Frequency", &noiseData.frequence, 0, 400);
		m_IsNewParametrs |= ImGui::SliderFloat("Amplitude", &noiseData.amplitude, 0, 1);
		m_IsNewParametrs |= ImGui::SliderFloat("Octaves", &noiseData.octaves, 0, 50);
		m_IsNewParametrs |= ImGui::SliderFloat("Vertex amplitude", &noiseData.multipler, 0, 100);
		m_IsNewParametrs |= ImGui::SliderFloat("exponent ", &noiseData.exponent, 0, 10);
		m_IsNewParametrs |= ImGui::SliderFloat("uv scale ", &noiseData.scale, 0, 100);
		ImGui::Text("Water");
		m_IsNewParametrs |= ImGui::SliderFloat("normal strength", &noiseData.normalStrength, 0, 1);
		m_IsNewParametrs |= ImGui::SliderFloat("water scale ", &noiseData.waterScale, 0, 5000);
		m_IsNewParametrs |= ImGui::SliderFloat("water depth ", &noiseData.waterDepth, 0, 100);
		m_IsNewParametrs |= ImGui::SliderFloat("water strength ", &noiseData.waterStrength, 0, 1);
		m_IsNewParametrs |= ImGui::ColorEdit3("deep water color ", &noiseData.deepWaterColor[0]);
		m_IsNewParametrs |= ImGui::ColorEdit3("shallow water color", &noiseData.shallowWaterColor[0]);
		ImGui::Text("Clouds");
		m_IsNewParametrs |= ImGui::SliderFloat("Cell amountA ",  &noiseData.cellAmountA, 0, 100);
		m_IsNewParametrs |= ImGui::SliderFloat("Cell amountB ",  &noiseData.cellAmountB, 0, 100);
		m_IsNewParametrs |= ImGui::SliderFloat("Cell amountC ",  &noiseData.cellAmountC, 0, 100);
		m_IsNewParametrs |= ImGui::SliderFloat("Cloud minus scale ", &noiseData.cloudScaleMinus, -100, 100);
		m_IsNewParametrs |= ImGui::SliderFloat("Cloud minus cell", &noiseData.cellAmountMinus, -100, 100);
		m_IsNewParametrs |= ImGui::SliderFloat("Persistence", &noiseData.persistence, 0, 1);

		m_IsNewParametrs |= ImGui::SliderFloat("Cell amountA LowRes ", &noiseData.numCellsALowRes, 0, 100);
		m_IsNewParametrs |= ImGui::SliderFloat("Cell amountB LowRes ", &noiseData.numCellsBLowRes, 0, 100);
		m_IsNewParametrs |= ImGui::SliderFloat("Cell amountC LowRes ", &noiseData.numCellsCLowRes, 0, 100);
		m_IsNewParametrs |= ImGui::SliderFloat("Persistence low res", &noiseData.persistenceLowRes, 0, 1);

		m_IsNewParametrs |= ImGui::SliderFloat("Density Offset", &cloudParams.densityOffset, -100, 100);
		m_IsNewParametrs |= ImGui::SliderFloat("Density Mult", &cloudParams.densityMult, 0, 10);
		m_IsNewParametrs |= ImGui::SliderFloat("Cloud Scale", &noiseData.cloudScale, 0, 100);
		m_IsNewParametrs |= ImGui::SliderFloat("Cloud Speed", &noiseData.cloudSpeed, -100, 100);
		m_IsNewParametrs |= ImGui::SliderFloat4("Weights", &cloudParams.weights[0], 0, 100);
		m_IsNewParametrs |= ImGui::SliderFloat3("Sun direction", &cloudParams.lightDir[0], -200, 200);
		m_IsNewParametrs |= ImGui::SliderFloat3("Sun position", &cloudParams.lightPos[0], -200, 200);
		m_IsNewParametrs |= ImGui::SliderFloat3("Cloud position", &cloudParams.cloudPos[0], -4500, 4500);
		m_IsNewParametrs |= ImGui::SliderFloat3("Cloud box Scale", &cloudParams.boxScale[0], -5000, 5000);
		cloudParams.boxScale[2] = cloudParams.boxScale[0];
		//cloudParams.boxScale[1] =cloudParams.boxScale[0] / 2;
		m_IsNewParametrs |= ImGui::SliderFloat("Light Absorption", &cloudParams.lightAbsorption, -10, 10);
		m_IsNewParametrs |= ImGui::SliderFloat("A hg", &cloudParams.aHg, -10, 10);
		m_IsPolygon = ImGui::Button("change mode");
		
	

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


	void Renderer::SubmitInstanceData(const InstanceData& instance)
	{
		m_InstanceData.emplace_back(instance);
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
		m_CubeBuffer.reset();
		m_IndexCubeBuffer.reset();

		m_Image.reset();
		m_NoiseImage.reset();
		m_AnimatedNoiseImage.reset();
		m_3DNoiseTexture.reset();
		m_3DNoiseTextureLowRes.reset();
		
		m_SnowTex.reset();
		m_GrassTex.reset();
		m_StoneTex.reset();
		m_WaterNormalImage.reset();
		m_WaterNormalImage2.reset();
		m_Cubemap.reset();

		m_InstancedDataBuffer.reset();
		m_CloudBuffer.reset();
		delete m_NoiseData;
		m_PointsData.reset();
		m_Device->GetDevice().freeMemory(m_MsaaImageMemory);
		m_Device->GetDevice().destroyImage(m_MsaaImage);
		m_Device->GetDevice().destroyImageView(m_MsaaImageView);

		//m_GraphicsCommandBuffer.GetCommandBuffer().reset();
		//m_GraphicsCommandBuffer.Free();

		for (size_t i = 0; i < m_Swapchain->m_SwapchainFrames.size(); i++) {
			delete m_UniformBuffers[i];
		}
		for (auto& buffer : m_ShaderStorageBuffers)
		{
			buffer.reset();
		}
		
		//delete m_DescriptorSetLayout;
		//delete m_DescriptorSetLayoutTex;
		m_DescriptorPool.reset();
		m_DescriptorPoolClouds.reset();
		m_DescriptorPoolTex.reset();
		m_DescriptorPoolNoise.reset();
	    m_DescriptorPoolSky.reset();
		m_Device->GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayoutSky->GetLayout());
		m_Device->GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayoutTex->GetLayout());
		m_Device->GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayoutNoise->GetLayout());
		m_Device->GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayoutClouds->GetLayout());
		m_Device->GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayout->GetLayout());
	

	
		
	
		
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
		device.destroyRenderPass(m_TerrainPipeline->m_RenderPass);
		m_TerrainPipeline.reset();
		m_WaterPipeline.reset();
		m_SkyPipeline.reset();
		m_RayMarchPipeline.reset();
		
		m_Swapchain.reset();

		m_Device->GetDevice().destroyPipeline(m_ComputePipeline);
		m_Device->GetDevice().destroyPipelineLayout(m_ComputePipelineLayout);
		m_Device->GetDevice().destroyPipeline(m_ComputePipelineClouds);
		m_Device->GetDevice().destroyPipelineLayout(m_ComputePipelineLayoutClouds);

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
