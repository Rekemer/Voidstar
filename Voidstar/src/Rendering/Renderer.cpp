
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

#include "Pipeline.h"
#include <random>
#include "Initializers.h"
#include "input.h"
#include  "Binder.h"
#include  "Generation.h"
#include"Settings.h"
#include"Sync.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>


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
	std::vector<Vertex> sphere;
	std::vector<IndexType> sphereIndicies;
	TracyVkCtx ctx;
	const int QUAD_AMOUNT = 700;




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
		m_BatchInstance = m_BatchInstanceStart;
	}

	void Renderer::DrawBatch(vk::CommandBuffer& commandBuffer,size_t offset, int index)
	{
		vk::DeviceSize offsets[] = { offset };

		{
			vk::Buffer vertexBuffers[] = { m_QuadBufferBatch->GetBuffer() };
			commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		}
		commandBuffer.bindIndexBuffer(m_QuadBufferBatchIndex->GetBuffer(), 0, m_QuadBufferBatchIndex->GetIndexType());
		commandBuffer.drawIndexed(m_QuadIndex, 1, 0, 0, 0);
	}
	void Renderer::DrawBatchCustom(vk::CommandBuffer& commandBuffer, size_t indexAmount,size_t offset, int index)
	{
		vk::DeviceSize offsets[] = { offset };

		{
			vk::Buffer vertexBuffers[] = { m_QuadBufferBatch->GetBuffer() };
			commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		}
		commandBuffer.bindIndexBuffer(m_QuadBufferBatchIndex->GetBuffer(), 0, m_QuadBufferBatchIndex->GetIndexType());
		commandBuffer.drawIndexed(indexAmount, 1, m_QuadIndex-index, 0, 0);
	}

	void UpdateVertex(Vertex*& vertex, glm::vec3 position, glm::vec4& color, glm::mat4& world,  int vertIndex, size_t texID =0)
	{
		vertex->Position = world * glm::vec4{ position,1 };
		vertex->UV = quad.verticies[vertIndex].UV;
		vertex->Color = color;
		vertex->textureID = texID;
	};
	void UpdateVerticies(Vertex*& vertex, std::vector<Vertex>& verticies)
	{
		UpdateVertex(vertex,verticies[0].Position, verticies[0].Color, glm::identity<glm::mat4>(),0,verticies[0].textureID);
		vertex++;																				   
		UpdateVertex(vertex,verticies[2].Position, verticies[2].Color, glm::identity<glm::mat4>(),2,verticies[0].textureID);
		vertex++;																				   
		UpdateVertex(vertex,verticies[3].Position, verticies[3].Color, glm::identity<glm::mat4>(),3,verticies[0].textureID);
		vertex++;																				   
		UpdateVertex(vertex,verticies[1].Position, verticies[1].Color, glm::identity<glm::mat4>(),1,verticies[0].textureID);
		vertex++;

	};
	void Renderer::DrawTxt(vk::CommandBuffer commandBuffer, std::string_view str, glm::vec2 pos, std::map< unsigned char, Character>& characters)
	{
		float scale = 1;
		float scaleX = 1;
		auto offset = pos;
		for (auto e : str)
		{

			if (e == '\n')
			{
				offset.x = pos.x;
				pos.y -= 3*Character::lineSpacing/64.f;
				continue;
			}
			else if (e == ' ')
			{
				offset.x += 5;
				continue;

			}
			else if (e == '\t')
			{
				offset.x += 30;
				continue;

			}
			if (characters.find(e) == characters.end()) continue;
		auto& characterData = characters.at(e);
		offset.x = offset.x + characterData.Bearing.x* scale;
		// to account for letter like p and q
		offset.y = pos.y - ( characterData.Size.y - characterData.Bearing.y)* scale;
		glm::vec4 color{ 1 };
		glm::mat4 world{ 1 };
		// left bottom
		m_BatchQuad->Position = glm::vec3{ offset.x ,offset.y,0};
		m_BatchQuad->UV = { characterData.minUv.x,characterData.maxUv.y };
		
		m_BatchQuad++;
		// right bottom
		m_BatchQuad->Position = glm::vec3{ offset.x + characterData.Size.x * scaleX ,offset.y,0 };
		m_BatchQuad->UV = { characterData.maxUv.x,characterData.maxUv.y };
		m_BatchQuad++;
		// right top
		m_BatchQuad->Position = glm::vec3{ offset.x + characterData.Size.x * scaleX,offset.y + characterData.Size.y * scale,0 };
		m_BatchQuad->UV = { characterData.maxUv.x,characterData.minUv.y };
		m_BatchQuad++;
		
		
		// left top
		m_BatchQuad->Position = glm::vec3{ offset.x ,offset.y + characterData.Size.y * scale,0 };
		m_BatchQuad->UV = { characterData.minUv.x,characterData.minUv.y };
		m_BatchQuad++;
		
		
		offset.x += characterData.Advance / 64.f* scaleX;
		m_QuadIndex += 6;
		}
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

	void Renderer::Draw(Sphere& sphere)
	{
		DrawSphere(sphere.Pos,sphere.Scale,sphere.Color,sphere.Rot);
	}
	void Renderer::Draw(QuadRangle& quadrangle)
	{
		DrawQuad(quadrangle.Verticies);
	}
	void Renderer::DrawSphere(glm::vec3 pos, glm::vec3 scale, glm::vec4 color, glm::vec3 rot)
	{
		m_BatchInstance->Color = color;
		std::cout << rot.x << " " << rot.y << std::endl;
		auto iden = glm::identity<glm::mat4>();
		iden = glm::translate(iden, pos);
		auto rotMatrix =  glm::rotate(iden, glm::radians(rot.x), glm::vec3{ 1,0,0 });
		rotMatrix = glm::rotate(rotMatrix, glm::radians(rot.y), glm::vec3{ 0,1,0 });
		rotMatrix = glm::scale(rotMatrix,scale);
		auto transpose = glm::transpose(rotMatrix);
		m_BatchInstance->WorldMatrix= transpose;
		m_BatchInstance++;
	}
	void Renderer::DrawSphereInstance(vk::CommandBuffer& commandBuffer)
	{
		vk::DeviceSize offsets[] = { 0 };

		{
			vk::Buffer vertexBuffers[] = { m_SphereBuffer->GetBuffer() };
			vk::Buffer instanceBuffers[] = { m_InstanceBuffer->GetBuffer() };
			commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
			commandBuffer.bindVertexBuffers(1, 1, instanceBuffers, offsets);

		}
		commandBuffer.bindIndexBuffer(m_SphereIndexBuffer->GetBuffer(), 0, m_SphereIndexBuffer->GetIndexType());
		auto instanceAmount = static_cast<uint64_t>(m_BatchInstance - m_BatchInstanceStart);
		commandBuffer.drawIndexed(m_SphereIndexBuffer->GetIndexAmount(), instanceAmount, 0, 0, 0);
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
		m_Window=window; 
		m_ViewportWidth = screenWidth;
		m_ViewportHeight = screenHeight;
		m_App = app;
		m_CommandPoolManager = CreateUPtr<CommandPoolManager>();
		// create instance
		CreateInstance();


		RenderContext::CreateSurface(window.get());
		RenderContext::CreateDevice();
		m_Device = RenderContext::GetDevice();

		RenderContext::CreateSwapchain(vk::Format::eB8G8R8A8Unorm,
			m_ViewportWidth, m_ViewportHeight,
			vk::PresentModeKHR::eFifo, vk::ColorSpaceKHR::eSrgbNonlinear) ;
		


		std::vector<vk::DescriptorPoolSize> pool_sizes =
		{
			{ vk::DescriptorType::eCombinedImageSampler, 10 },
			{ vk::DescriptorType::eStorageImage, 10 },
			{ vk::DescriptorType::eStorageBuffer, 10 },
			{ vk::DescriptorType::eInputAttachment, 10 },
			{ vk::DescriptorType::eUniformBuffer, 10 },
		};

		m_UniversalPool = DescriptorPool::Create(pool_sizes, 10);

		



		auto commandBufferInit = [this]()
		{
			auto frameAmount = RenderContext::GetFrameAmount();
			m_FrameCommandPool = Renderer::Instance()->GetCommandPoolManager()->GetFreePool();
			m_RenderCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, frameAmount	);
			m_TransferCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, frameAmount);
			m_ComputeCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, frameAmount);
		};
		commandBufferInit();
		
		quad = GeneratePlane(1);
		sphere = GenerateSphere(1,10, sphereIndicies);
		auto& verticies = quad.verticies;
		auto& indices = quad.indicies;
		auto indexSize = SizeOfBuffer(indices.size(), indices[0]);
		{
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
			}
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


		{
			auto indexSize = SizeOfBuffer(sphereIndicies.size(), sphereIndicies[0]);
			{
				{
					SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(indexSize);


					{
						BufferInputChunk inputBuffer;
						inputBuffer.size = indexSize;
						inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
						inputBuffer.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
						m_SphereIndexBuffer = CreateUPtr<IndexBuffer>(inputBuffer, sphereIndicies.size(), vk::IndexType::eUint32);

					}

					m_TransferCommandBuffer[0].BeginTransfering();
					m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_SphereIndexBuffer.get(), (void*)sphereIndicies.data(), indexSize);
					m_TransferCommandBuffer[0].EndTransfering();
					m_TransferCommandBuffer[0].SubmitSingle();
				}
				{
					auto vertexSize = SizeOfBuffer(sphere.size(), sphere[0]);
					{
						BufferInputChunk inputBuffer;
						inputBuffer.size = vertexSize;
						inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
						inputBuffer.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;

						m_SphereBuffer = CreateUPtr<Buffer>(inputBuffer);
					}
					SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(vertexSize);
					m_TransferCommandBuffer[0].BeginTransfering();
					m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_SphereBuffer.get(), (void*)sphere.data(), vertexSize);
					m_TransferCommandBuffer[0].EndTransfering();
					m_TransferCommandBuffer[0].SubmitSingle();
				}
			}
		}
		

		{
			BufferInputChunk inputBuffer;
			inputBuffer.size = sizeof(InstanceData) * 30;
			inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			inputBuffer.usage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eVertexBuffer;
			m_InstanceBuffer = CreateUPtr<Buffer>(inputBuffer);
			m_BatchInstanceStart = reinterpret_cast<InstanceData*>(m_Device->GetDevice().mapMemory(m_InstanceBuffer->GetMemory(), 0, inputBuffer.size));

		}

		// get frame amount
		auto framesAmount = RenderContext::GetFrameAmount();
		auto m_Device = RenderContext::GetDevice();
		auto bufferSize = sizeof(UniformBufferObject);
		m_UniformBuffers.resize(framesAmount);
		uniformBuffersMapped.resize(framesAmount);


		BufferInputChunk inputBuffer;
		inputBuffer.size = bufferSize;
		inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		inputBuffer.usage = vk::BufferUsageFlagBits::eUniformBuffer;

		for (size_t i = 0; i < framesAmount; i++)
		{
			m_UniformBuffers[i] = CreateUPtr<Buffer>(inputBuffer);
			uniformBuffersMapped[i] = m_Device->GetDevice().mapMemory(m_UniformBuffers[i]->GetMemory(), 0, bufferSize);
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


		
	}

	
	

	static std::unordered_map<ShaderType, const char*> PipelineShaderFolders =
	{
		{ShaderType::VERTEX,"Vertex"},
		{ShaderType::FRAGMENT,"Fragment"},
		{ShaderType::TESS_CONTROL,"Tesselation"},
		{ShaderType::TESS_EVALUATION,"Tesselation"},
		{ShaderType::COMPUTE,"Compute"}
	};
	static std::unordered_map<ShaderType, const char*> PipelineShaderExtensions =
	{
		{ShaderType::VERTEX,".vert"},
		{ShaderType::FRAGMENT,".frag"},
		{ShaderType::TESS_CONTROL,".tesc"},
		{ShaderType::TESS_EVALUATION,".tese"},
		{ShaderType::COMPUTE,".comp"}
	};
	static std::unordered_map<ShaderType, const char*> PipelineShaderBinaryExtensions =
	{
		{ShaderType::VERTEX,".spvV"},
		{ShaderType::FRAGMENT,".spvF"},
		{ShaderType::TESS_CONTROL,".spvC"},
		{ShaderType::TESS_EVALUATION,".spvE"},
		{ShaderType::COMPUTE,".spvCmp"}
	};

	std::string GetFileNameWithoutExtension(const std::string& filepath)
	{
		size_t extensionIndex = filepath.find_last_of('.');
		return filepath.substr(0, extensionIndex);
	}

	std::string CreateCommand(std::string_view shader, const char* binaryExtension, std::string& shaderPath)
	{
		auto name = GetFileNameWithoutExtension(shader.data());

		std::string shaderOutput = BASE_SPIRV_OUTPUT + name.c_str() + binaryExtension;
		std::string command = SPIRV_COMPILER_PATH + " -V " + shaderPath + " -o " + shaderOutput;
		return command;
	}

	void Renderer::CompileShader(std::string_view binaryShaderName, ShaderType type)
	{	
		auto folder = PipelineShaderFolders[type];
		auto shaderName = GetFileNameWithoutExtension(binaryShaderName.data());
		shaderName += PipelineShaderExtensions[type];
		auto path = BASE_SHADER_PATH +  folder + "/" + shaderName.data();
		auto isExist = std::filesystem::exists(path);
		if (isExist)
		{
			auto binaryExtension = PipelineShaderBinaryExtensions[type];
			auto command = CreateCommand(shaderName, binaryExtension , path);
			int result = std::system(command.c_str());
			if (result != 0)
			{
				Log::GetLog()->error("shader {0} is not compiled! ", shaderName.data());
			}
		}
		else
		{
			Log::GetLog()->error("SHADER COMPILATOIN: Path {0} is not found ", path);
		}
	}
	void Renderer::Draw(Drawable& drawable)
	{
		drawable.m_Self->Draw();
	}
	void Renderer::UserInit()
	{
		m_UserFunctions.bindingsInit();
		CreateLayouts();
		AllocateSets();
		m_UserFunctions.createResources();
		m_UserFunctions.bindResources();
		m_UserFunctions.createPipelines();

	}
	void Renderer::CreateSyncObjects()
	{	
		auto frameAmount = RenderContext::GetFrameAmount();
		m_ComputeInFlightFences.resize(frameAmount);
		m_ComputeFinishedSemaphores.resize(frameAmount);
		m_ImageAvailableSemaphore.resize(frameAmount);
		m_RenderFinishedSemaphore.resize(frameAmount);
		m_InFlightFence.resize(frameAmount);
	
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


		SwapChainSupportDetails support;
		auto device = RenderContext::GetDevice();
		auto surface = RenderContext::GetSurface();
		support.AvailableCapabilities = device->GetDevicePhys().getSurfaceCapabilitiesKHR(*surface);
		support.AvailablePresentModes = device->GetDevicePhys().getSurfacePresentModesKHR(*surface);
		support.AvailableFormats = device->GetDevicePhys().getSurfaceFormatsKHR(*surface);
		support.ViewportWidth = m_ViewportWidth;
		support.ViewportHeight = m_ViewportHeight;
		
		RenderContext::RecreateSwapchain(support);
		
		auto& camera = m_App->GetCamera();
		camera->UpdateProj(m_ViewportWidth, m_ViewportHeight);
	}

	void Renderer::Shutdown()
	{

			
			


			auto device = m_Device->GetDevice();
			device.waitIdle();



			TracyVkDestroy(ctx)

			m_CommandPoolManager->FreePool(m_TracyCommandPool);

				std::for_each(m_Graphs.begin(),
					m_Graphs.end(),
					[](UPtr<RenderPassGraph>& graph)
					{
						graph->Destroy();
					});

			for (int i = 0; i < m_ComputeCommandBuffer.size(); i++)
			{
				m_RenderCommandBuffer[i].Free();
				m_ComputeCommandBuffer[i].Free();
				m_TransferCommandBuffer[i].Free();
			};

			Renderer::Instance()->GetCommandPoolManager()->FreePool(m_FrameCommandPool);



			m_UniformBuffers.clear();
			m_UniversalPool.reset();
			CleanUpLayouts();
			m_ImageAvailableSemaphore.clear();
			m_RenderFinishedSemaphore.clear();
			m_ComputeFinishedSemaphores.clear();
			m_InFlightFence.clear();
			m_ComputeInFlightFences.clear();
			m_ImageAvailableSemaphore.clear(); 


			m_Pipelines.clear();
			m_CommandPoolManager->Release();
			m_QuadBuffer.reset();
			m_SphereBuffer.reset();
			m_InstanceBuffer.reset();
			m_SphereIndexBuffer.reset();
			m_QuadIndexBuffer.reset();
			m_QuadBufferBatch.reset();
			m_QuadBufferBatchIndex.reset();
			RenderContext::Shutdown();
			


		
	}


	void Renderer::RecordCommandBuffer(uint32_t imageIndex,vk::RenderPass& renderPass,vk::Pipeline& pipeline, vk::PipelineLayout& pipelineLayout, int instances)
	{

		
	}

	
	Renderer* Renderer::Instance()
	{
		static Renderer renderer ;
		return &renderer;
	}
	void Renderer::Render(float deltaTime,Camera& camera)
	{
		auto exeTime = m_App->GetExeTime();
		uint32_t imageIndex;
		auto swapchain = RenderContext::GetSwapchain();
		{
			ZoneScopedN("Acquiring new Image");
			m_Device->GetDevice().acquireNextImageKHR(swapchain->m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphore[m_CurrentFrame].GetSemaphore(), nullptr, &imageIndex);
		}
		vk::Semaphore renderFinished;
		renderFinished = m_Graphs[0]->Execute(m_RenderCommandBuffer[m_CurrentFrame], m_CurrentFrame, m_ImageAvailableSemaphore[m_CurrentFrame]);
		
		
		auto& semaphore = m_ImageAvailableSemaphore[m_CurrentFrame].GetSemaphore();
		vk::Semaphore waitSemaphore[] = { renderFinished};
	
			

		

		vk::PresentInfoKHR presentInfo = {};
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = waitSemaphore;

		vk::SwapchainKHR swapChains[] = { swapchain->m_Swapchain };
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

		m_CurrentFrame = (m_CurrentFrame + 1) % RenderContext::GetFrameAmount();

		
		
		FrameMark;
	}


	

	


	

	
	void Renderer::Wait(vk::Fence& fence)
	{
		m_Device->GetDevice().waitForFences(fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
	}
	void Renderer::Reset(vk::Fence& fence)
	{
		m_Device->GetDevice().resetFences(fence);
	}

	
	void Renderer::Flush(std::vector<vk::CommandBuffer> commandBuffers)
	{
		assert(false);


	}
	void Renderer::UpdateUniformBuffer(const glm::mat4& proj,Camera& camera)
	{
		UniformBufferObject ubo{};

		auto cameraView = camera.GetView();
		auto cameraProj = camera.GetProj();
		ubo.view = cameraView;
		ubo.proj = cameraProj;
		ubo.time = m_App->GetExeTime();
		memcpy(uniformBuffersMapped[m_CurrentFrame], &ubo, sizeof(ubo));
		//auto ans = cameraProj * cameraView * glm::vec4{ 1,0,1,1 };
		//ans /= ans.w;
		//std::cout << "sd";

	}

	void Renderer::BeginFrame(Camera& camera, size_t viewportWidth,
		size_t viewportHeight)
	{
		auto proj = glm::ortho(0.0f, (float)viewportWidth, (float)viewportHeight,0.f);
		UpdateUniformBuffer(camera.GetProj(), camera);
	}
	void Renderer::EndFrame()
	{
		for (auto& e : m_Drawables)
		{
			e.second.clear();
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


	

	Renderer::~Renderer()
	{

	}

	


	
	void Renderer::CreateSurface()
	{
		
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
		RenderContext::CreateInstance(info);
		m_Instance = RenderContext::GetInstance();
		
	}
	
}
