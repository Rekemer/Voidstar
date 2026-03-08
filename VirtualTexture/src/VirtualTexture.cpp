

#include <iostream>
#include <vector>

#include"Voidstar.h"
#include"tracy/Tracy.hpp"
#include"tracy/TracyVulkan.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H  
#include <map>
#include <utility>
#include <functional>
#include <future>
#include "gtx/compatibility.hpp"
using namespace Voidstar;


#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"


#include <spirv_cross/spirv_cross.hpp>
#include "Vertex.h"
#include "Rendering/Generation.h"
#include "Jobs.h"
#include "Cache.h"
#include "Rendering/Image.h"
#include "Rendering/Renderer.h"



constexpr int pageWidth = 128;
constexpr int pageHeight = 64;
constexpr int workingSetWidth = pageWidth * 20;
constexpr int workingSetHeight = pageHeight * 20;
constexpr int workingSetPageAmountX = workingSetWidth / pageWidth ;
constexpr int workingSetPageAmountY = workingSetHeight / pageHeight;
constexpr int workingSetPageAmount = workingSetPageAmountX * workingSetPageAmountY;
constexpr float low = 32768;
constexpr glm::vec2 virtualTextureSize = { low*2,low};
constexpr glm::vec2 virtualTextureTiles = virtualTextureSize / glm::vec2{pageWidth,pageHeight};
//constexpr glm::vec2 virtualTextureSize = { 1024,512 };
constexpr int pageTableWidth = virtualTextureSize.x/ pageWidth;
constexpr int pageTableHeight = virtualTextureSize.y / pageHeight;
uint32_t pageTableMipLevels = std::log2(std::max(pageTableWidth, pageTableHeight));
std::string_view RENDER_BASIC_PASS = "Basic";
std::string_view RENDER_DEBUG_PASS = "Debug";
std::string_view COMPUTE_PAGE_TABLE_PASS = "PageTable";
std::string_view COMPUTE_PAGE_TABLE_FINAL_PASS = "PageTableFinal";
std::string_view FEEDBACK_RENDER_PASS = "Feedback";
std::string_view IMGUI_RENDER_PASS = "ImGui";


#define execute(PipelineName)\
			[this](CommandBuffer& commandBuffer, size_t frameIndex)\
			{\
				Renderer::Instance()->BeginBatch();\
				auto vkCommandBuffer = commandBuffer.GetCommandBuffer();\
				auto pipeline = Renderer::Instance()->GetPipeline(PipelineName);\
				vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 0, m_DescriptorSets[frameIndex], nullptr);\
				vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 1, m_DescriptorSetTex, nullptr);\
				vkCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());\
				vk::Viewport viewport;\
				viewport.x = 0;\
				viewport.y = 0;\
				viewport.minDepth = 0;\
				viewport.maxDepth = 1;\
				viewport.width = Application::GetScreenWidth();\
				viewport.height = Application::GetScreenHeight();\
				vk::Rect2D scissors;\
				scissors.offset = vk::Offset2D{ (uint32_t)0,(uint32_t)0 };\
				scissors.extent = vk::Extent2D{ (uint32_t)viewport.width,(uint32_t)viewport.height};\
				vkCommandBuffer.setViewport(0, 1, &viewport);\
				vkCommandBuffer.setScissor(0, 1, &scissors);\
				auto& drawables = Renderer::Instance()->GetDrawables(PipelineName);\
				for (auto& quad : drawables)\
				{\
					Renderer::Instance()->Draw(quad);\
				}\
				Renderer::Instance()->DrawBatch(vkCommandBuffer);\
			}

#define OLD 0
class ExampleApplication : public Voidstar::Application
{
public:

	glm::vec2 feedbackSize;


	struct TileResult
	{
		int layer;
		Memory mem;
		size_t width = 0, height = 0;
		std::atomic<bool> ready{ false };
	};

	ExampleApplication(std::string appName, size_t screenWidth, size_t screenHeight) : Voidstar::Application(appName, screenWidth, screenHeight)
	{
		m_ComputeShaders[0] = LoadComputeProgram("pageTable.comp");
		m_ComputeShaders[1] = LoadComputeProgram("pageTableFinal.comp");



		m_FeedbackShader = LoadProgram("feedback.vert", "feedback.frag");
		m_DefaultShader = LoadProgram("basic.vert", "texture.frag");
		m_FinalShader = LoadProgram("render_screen_quad.vert","render_attachment.frag");
		m_FinalShaderWorkingSet = LoadProgram("feedback.vert","render_working_set.frag");
		m_TestTexture = LoadTexture("coffee.jpg");
		m_TestTexture1 = LoadTexture("dos_2_noise.png");

		m_VertexLayout.AddVertex(ShaderDataType::FLOAT3)
			.AddVertex(ShaderDataType::FLOAT2);
		auto [verts, indices] = GeneratePlane<Vertex>(1);
		m_Plane.verticies = verts;
		m_Plane.indicies= indices;


		
		
		Memory mem;
		mem.size = sizeof(PageEntry) * virtualTextureTiles.x * virtualTextureTiles.y;
		m_StorageBuffers = CreateBuffer(mem,ResourceUsage::StorageRead | ResourceUsage::StorageWrite | ResourceUsage::Readback);

		



		m_PageTable = CreateEmptyTexture(pageTableWidth, pageTableHeight, TextureFormat::RGBA32_SFLOAT, ResourceUsage::Sampled | ResourceUsage::TransferDst | ResourceUsage::TransferSrc
			| ResourceUsage::StorageRead |
			ResourceUsage::StorageWrite);
		m_PageTableMipMaps = GenerateMipMapsAsTextures(m_PageTable, pageTableMipLevels);
		int bufferOffset = 0;
		ExecuteFrame(0);
		ExecuteFrame(0);
		auto mipMapSize = 0;
		for (auto mipMap : m_PageTableMipMaps)
		{
			mipMapSize += GetSize(mipMap);
		}
		{
			Memory mem;
			mem.size = pageTableWidth * pageTableHeight * 8 * 4 + mipMapSize;
			m_FillBuffer = CreateBuffer(mem, ResourceUsage::TransferSrc| ResourceUsage::TransferDst| ResourceUsage::Upload);
		}

		FillImage(m_PageTable, glm::vec4{ -1, -1, -1, -1 },m_FillBuffer, bufferOffset);

		for (auto mipMaps : m_PageTableMipMaps)
		{
			bufferOffset += GetSize(mipMaps);
			FillImage(mipMaps, glm::vec4{ -1, -1, -1, -1 }, m_FillBuffer, bufferOffset);
		}


		//ExecuteFrame(0);
		auto usage = ResourceUsage::Sampled | ResourceUsage::TransferDst;
		m_WorkingSet = CreateEmptyTexture(pageWidth, pageHeight,TextureFormat::RGBA8_UNORM, usage, 1, SampleCount::e1, FilterMode::Linear, FilterMode::Linear, workingSetPageAmount, false);



		auto& planeV = m_Plane.verticies;
		auto& planeI = m_Plane.indicies;
		m_Vertexhandle = CreateVertexBuffer({
			reinterpret_cast<uint8_t*>(planeV.data()),planeV.size() * sizeof(planeV[0]) }
		, m_VertexLayout);
		m_IndexHandle = CreateIndexBuffer
		(
			Memory{ reinterpret_cast<uint8_t*>(planeI.data()), planeI.size() * sizeof(planeI[0]) }
		);


		feedbackSize = { Application::GetScreenWidth(), Application::GetScreenHeight()};
		feedbackSize = { Application::GetScreenWidth() / 70 ,Application::GetScreenHeight() / 70 };


		m_FeedbackAttachments[0] = CreateAttachment(AttachmentType::COLOR,TextureFormat::RGBA32_SFLOAT,
			feedbackSize.x,feedbackSize.y, SampleCount::e1, AttachmentHint::SampledLater | AttachmentHint::Readback);
		m_FeedbackFramebuffer = CreateFramebuffer( {m_FeedbackAttachments[0]});

	
		
		ExecuteFrame(0);
		ExecuteFrame(0);
		data = std::make_unique<uint8_t[]>(sizeof(FeedbackRes) * feedbackSize.x * feedbackSize.y);
		m_FeedbackRes.resize(feedbackSize.x * feedbackSize.y);
		m_Clear = std::vector<PageEntry>(virtualTextureTiles.x * virtualTextureTiles.y);
	}
	~ExampleApplication()
	{

	}
	void PreUpdate(Camera& camera) override
	{
			
	}

	void Update(float deltaTime) override
	{

		// feedback pass

		auto feedbackProj = glm::perspectiveRH_ZO(glm::radians(75.0f), float(Application::GetScreenWidth() / Application::GetScreenHeight()), 0.0001f, 1000.0f);
		SetViewRect(m_FeedbackRenderPass, 0, 0, feedbackSize.x, feedbackSize.y);
		SetFramebuffer(m_FeedbackRenderPass, m_FeedbackFramebuffer);
		SetViewTransform(m_FeedbackRenderPass, GetCamera()->GetView(), feedbackProj);
		BindVertexBuffer(0, m_Vertexhandle);
		BindIndexBuffer(m_IndexHandle);
		Submit(m_FeedbackRenderPass, m_FeedbackShader);


		auto scene = GetColorTexture(m_FeedbackFramebuffer);
		//BindAttachmentAsTexture("u_Scene", scene);
		
		//SetViewRect(m_FinalRenderPass, 0, 0, Application::GetScreenWidth(), Application::GetScreenHeight());
		//SetViewTransform(m_FinalRenderPass, GetCamera()->GetView(), GetCamera()->GetProj());
		//Submit(m_FinalRenderPass, m_FinalShader);
		
		//ExecuteFrame(deltaTime);
		ExecuteFrame(deltaTime);
		ExecuteFrame(0);

		ReadTexture(scene, data.get());
		ExecuteFrame(deltaTime);

	#if 1

			static uint64_t lastProcessed = 0;

			//ExecuteFrame(deltaTime);
		
			auto currentFrame = GetCurrentFrame();
			//if (frameWait > lastProcessed)
			//{
			
				auto ptr = reinterpret_cast<float*>(data.get());
				uint64_t bufferSize = sizeof(FeedbackRes) * feedbackSize.x * feedbackSize.y;

				for (int i = 0, memoryRead = 0; memoryRead < bufferSize; memoryRead += sizeof(FeedbackRes), i++)
				{
					auto r = *(float*)(ptr);
					ptr++;
					auto g = *(float*)(ptr);
					ptr++;
					auto b = *(float*)(ptr);
					ptr++;
					auto a = *(float*)(ptr);
					ptr++;
					m_FeedbackRes[i] = { r,g,b,a };
				}
			//}
			static std::unordered_map<int, std::string_view> mipTiles =
			{
				{9,"pages_65536_32768/"},
				{8,"pages_32768_16384/"},
				{7,"pages_16384_8192/"},
				{6,"pages_8192_4096/"},
				{5,"pages_4096_2048/"},
				{4,"pages_2048_1024/"},
				{3,"pages_1024_512/"},
				{2,"pages_512_256/"},
				{1,"pages_256_128/"},
				{0,"pages_128_64/"},
			};




			// so we can go back to low res mip level
			// have tiles that
			// should not have local vectors 
			m_TilesWeSee.clear();
			{
				std::vector<SPtr<TileResult>> tilesToLoad;
				ZoneScopedN("Reading Feedback Buffer");
				for (auto& feedback : m_FeedbackRes)
				{
					//	// there is feedback
					if (feedback.isValid > 0)
					{
						std::stringstream ss;

						ss << (int)feedback.pageX << "_" << (int)feedback.pageY << ".png";

						std::string path = BASE_VIRT_PATH + mipTiles[feedback.mipMap].data() + ss.str();
						// check cache instead
						auto cachedPage = m_Cache.Get(path);


						if (!cachedPage)
						{
							if (m_Overload)
							{
								ZoneScopedN("Replace old page");
								auto coords = m_Cache.GetLUPage(mipTiles, BASE_VIRT_PATH);
								m_WorkingSetPtr[0] = coords.x;
								m_WorkingSetPtr[1] = coords.y;
							}
							else
							{
								ZoneScopedN("Add new page");
								if (workingSetPageAmountX <= m_WorkingSetPtr[0] + 1)
								{
									m_WorkingSetPtr[0] = 0;
									if (workingSetPageAmountY <= m_WorkingSetPtr[1] + 1)
									{
										ZoneScopedN("Replace old page first time");
										// we dont have enough space, must overwrite something
										m_Overload = true;
										auto coords = m_Cache.GetLUPage(mipTiles, BASE_VIRT_PATH);
										m_WorkingSetPtr[0] = coords.x;
										m_WorkingSetPtr[1] = coords.y;
									}
									else
									{
										m_WorkingSetPtr[1] += 1;
									}
								}
								else
								{
									m_WorkingSetPtr[0] += 1;
								}
							}

							vk::Offset3D offset{ m_WorkingSetPtr[0],m_WorkingSetPtr[1] ,0 };
							int layer = m_WorkingSetPtr[1] * workingSetPageAmountX + m_WorkingSetPtr[0];
							assert(layer < workingSetPageAmount);
							std::cout << path << std::endl;

							auto result = CreateSPtr<TileResult>();
							result->layer = layer;
							

							m_Jobs.PushJob([result, path]() {
								size_t width = 0, height = 0;
								result->mem= Image::LoadImageCPU(path, width, height);
								result->width = width;
								result->height= height;
								result->ready = true;
								});

							
							tilesToLoad.push_back(result);

							float workingSetCoordX = (float)(m_WorkingSetPtr[0]);
							float workingSetCoordY = (float)m_WorkingSetPtr[1];
							glm::vec2 physCoord = { workingSetCoordX,workingSetCoordY };
							auto mipMap = feedback.mipMap;
							glm::vec2 pageCoord = { feedback.pageX ,feedback.pageY };
							PageEntry page{ mipMap,pageCoord,physCoord };

							m_Cache.Add(page, path);
							m_TilesWeSee.push_back(page);
						}
						else
						{
							ZoneScopedN("Add exisiting page");
							if (std::find(m_TilesWeSee.begin(), m_TilesWeSee.end(), *cachedPage) == m_TilesWeSee.end())
							{
								m_TilesWeSee.push_back(*cachedPage);
							}
						}

					}
				}
		
				for (auto it = tilesToLoad.begin();
					it != tilesToLoad.end(); )
				{
					auto& r = *it;
					if (r->ready)
					{
						UpdateImageRegionWithImage(r->mem, r->width, r->height, m_WorkingSet, {0,0,0}, r->layer);
						it++;
					}
					else
					{
						continue;
					}
				}
			}		

			if (m_TilesWeSee.size() > 0)
			{

				ZoneScopedN("Update visible pages");

			    
				SetData(m_StorageBuffers, m_Clear.data(), m_Clear.size() * sizeof(m_Clear.at(0)));
				SetData(m_StorageBuffers, m_TilesWeSee.data(), m_TilesWeSee.size() * sizeof(m_TilesWeSee.at(0)));

			}
		

			{
				ZoneScopedN("Init page table");
				int bufferOffset = 0;
				//transferBuffer.BeginTransfering();
				FillImage(m_PageTable, glm::vec4{ -1, -1, -1, -1 }, m_FillBuffer, bufferOffset);

				for (auto mipMaps : m_PageTableMipMaps)
				{
					bufferOffset += GetSize(mipMaps);
					FillImage(mipMaps, glm::vec4{ -1, -1, -1, -1 }, m_FillBuffer, bufferOffset);
				}
			
		

			}




	#endif
	
		auto& images = m_PageTableMipMaps;
		//images.push_back(m_PageTable);

		BindImages("storageImage", images);
		BindBuffer("u_StorageBuffer",m_StorageBuffers);
		SubmitCompute(m_UpdatePageTablePass[0], m_ComputeShaders[0], m_TilesWeSee.size(), 1, 1);
		BindImages("storageImage", m_PageTableMipMaps);
		BindImage("final", m_PageTable);
		SubmitCompute(m_UpdatePageTablePass[1], m_ComputeShaders[1], pageTableWidth, pageTableHeight, 1);

		
		SetViewRect(m_FinalRenderPass, 0, 0, Application::GetScreenWidth(), Application::GetScreenHeight());
		SetViewTransform(m_FinalRenderPass, GetCamera()->GetView(), GetCamera()->GetProj());
		BindVertexBuffer(0, m_Vertexhandle);
		BindIndexBuffer(m_IndexHandle);
		BindTexture("PageTable", m_PageTable);
		BindTexture("WorkingSet", m_WorkingSet);
		
		Submit(m_FinalRenderPass, m_FinalShaderWorkingSet);
		
		ExecuteFrame(deltaTime);

			
	}
	
	



private:

	PassID m_FeedbackRenderPass = 0;
	PassID m_UpdatePageTablePass[2] = {1,2};
	PassID m_FinalRenderPass = 3;
	PassID m_DebugRenderPass = 4;
	PassID m_CubeRenderPass = 5;
	UPtr<uint8_t[]>data;
	std::vector<size_t> ticket{ 3, 0 };
	ProgramHandle m_FeedbackShader;
	ProgramHandle m_ComputeShaders[2];
	ProgramHandle m_FinalShader;
	ProgramHandle m_FinalShaderWorkingSet;
	ProgramHandle m_DebugShader;

	BufferHandle m_StorageBuffers;
	BufferHandle m_FillBuffer;
	VertexBufferHandle m_Vertexhandle;
	IndexBufferHandle m_IndexHandle;
		
	ProgramHandle m_DefaultShader;
	VertexLayout m_VertexLayout;
	
	TextureHandle m_TestTexture;
	TextureHandle m_TestTexture1;
	UniformHandle m_TextureUniform;
	FrameBufferHandle m_FeedbackFramebuffer;
	AttachmentHandle m_FeedbackAttachments[2];
	std::vector<Vertex> m_Cube;
	std::vector<IndexType> m_IndexCube;
	std::vector<PageEntry> m_TilesWeSee;
	std::vector<PageEntry> m_Clear;

	int m_BaseDesc = 0;
	int m_PageTableDescCompute = 0;
	int m_PageTableDescFinalCompute= 0;
	int m_WorkingSetDesc = 0;
	int m_DebugTexturesDesc = 0;
	const uint32_t MAX_POINTS = 20;
	int nextPoint = 0;
	std::vector<vk::DescriptorSet> m_DescriptorSets;
	vk::DescriptorSet  m_PageTableDescriptorSet;
	vk::DescriptorSet  m_PageTableFinalDescriptorSet;

	UPtr<Buffer> m_AddInfo;
	std::vector<SPtr<Buffer>> m_StageBuffers;
	vk::DescriptorSet m_DescriptorSetDebug;
	vk::DescriptorSet m_DescriptorSetWorkingSet;
	glm::vec2 m_Follow;
	SPtr<Image> m_Image;
	int m_WorkingSetPtr[2] = {-1,0};
	bool m_Overload = false;
	TextureHandle m_WorkingSet;
	TextureHandle m_PageTable;
	std::vector<TextureHandle> m_PageTableMipMaps;


	std::vector<glm::vec2> m_ClickPoints;

	
	bool isClicked = false;
	bool isDragged = false;

	struct FeedbackRes
	{
		float pageX, pageY, mipMap, isValid;
	};
	std::vector<FeedbackRes> m_FeedbackRes;

	QuadData<Vertex> m_Plane;
	glm::mat4 iden{ 1 };
	glm::vec3  m_SphereRot = {0,0,0};
	Cache<workingSetPageAmount> m_Cache;
};



Voidstar::Application* Voidstar::CreateApplication()
{
	auto str = std::string("Example");
	// 120 -> 1920 * 1080
	// 110
	const int res = 110;
	return new ExampleApplication(str, std::min(16 * res,1920), std::min(9 * res, 1061));
}

int main()
{
	return Main();
}
