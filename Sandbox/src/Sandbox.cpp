﻿

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

// ImGui
static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;

void CleanUpImGui()
{
	auto device = RenderContext::GetDevice()->GetDevice();
	device.destroyDescriptorPool(g_DescriptorPool);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

std::map<unsigned char, Character> Characters;
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


class ExampleApplication : public Voidstar::Application
{
public:

	glm::vec2 feedbackSize;
	ExampleApplication(std::string appName, size_t screenWidth, size_t screenHeight) : Voidstar::Application(appName, screenWidth, screenHeight)
	{
		
		Settings params{3};
		m_ClickPoints.resize(MAX_POINTS, glm::vec2(-1, -1));

		feedbackSize = { Application::GetScreenWidth() / 70 ,Application::GetScreenHeight() / 70 };

		auto bindingsInit = [this]()
		{
			

			auto binderRender = Binder<RENDER>();
			m_BaseDesc = binderRender.BeginBind(3);
			binderRender.Bind(0, 1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eTessellationControl
				| vk::ShaderStageFlagBits::eTessellationEvaluation | vk::ShaderStageFlagBits::eFragment);
			m_DebugTexturesDesc= binderRender.BeginBind();
			binderRender.Bind(0, 1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
			binderRender.Bind(1, 1, vk::DescriptorType::eUniformBuffer,  vk::ShaderStageFlagBits::eFragment);
			m_WorkingSetDesc = binderRender.BeginBind();
			binderRender.Bind(0, 1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
			binderRender.Bind(1, 1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);

			auto binderCompute = Binder<COMPUTE>();
			m_PageTableDescCompute = binderCompute.BeginBind();
			binderCompute.Bind(0, pageTableMipLevels+1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);
			binderCompute.Bind(1, 1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
			m_PageTableDescFinalCompute = binderCompute.BeginBind();
			binderCompute.Bind(0, pageTableMipLevels, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);
			binderCompute.Bind(1, 1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);
		};

		auto createResources= [this]()
		{
			
			{
				BufferInputChunk inputBuffer;
				inputBuffer.size = sizeof(AdditionalData);
				inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
				inputBuffer.usage = vk::BufferUsageFlagBits::eUniformBuffer;
				m_AddInfo = CreateUPtr<Buffer>(inputBuffer);
			}
			
			{
				BufferInputChunk inputBuffer;
				inputBuffer.size = sizeof(FeedbackRes) * virtualTextureTiles.x*virtualTextureTiles.y;
				inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
				inputBuffer.usage = vk::BufferUsageFlagBits::eStorageBuffer;
				m_StorageBuffers = CreateUPtr<Buffer>(inputBuffer);
			}
		
			
			auto usage =  vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;

			m_WorkingSet = Image::CreateEmptyImage(pageWidth, pageHeight, vk::Format::eR8G8B8A8Unorm, usage,1,
				vk::SampleCountFlagBits::e1,vk::Filter::eLinear, vk::Filter::eLinear, workingSetPageAmount, vk::ImageViewType::e2DArray);

			
			auto commandBuffer = Renderer::Instance()->GetTransferCommandBuffer(0);


			commandBuffer.BeginTransfering();
			commandBuffer.ChangeImageLayout(m_WorkingSet.get(), vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,1,workingSetPageAmount);
			commandBuffer.EndTransfering();
			commandBuffer.SubmitSingle();


			
			m_PageTable = Image::CreateEmptyImage(pageTableWidth, pageTableHeight, vk::Format::eR32G32B32A32Sfloat, usage
				| vk::ImageUsageFlagBits::eTransferSrc| vk::ImageUsageFlagBits::eStorage |vk::ImageUsageFlagBits::eTransferDst);
			

			commandBuffer.BeginTransfering();
			commandBuffer.ChangeImageLayout(m_PageTable.get(), vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
			commandBuffer.EndTransfering();
			commandBuffer.SubmitSingle();

			commandBuffer.Free();
			
			//commandBuffer.BeginTransfering();
			//commandBuffer.ChangeImageLayout(m_PageTable.get(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, pageTableMipLevels);
			//commandBuffer.EndTransfering();
			//commandBuffer.SubmitSingle();
			m_PageTableMipMaps = m_PageTable->GenerateEmptyMipmapsAsImages(pageTableMipLevels);
			auto mipMapSize = 0;
			for (auto mipMap : m_PageTableMipMaps)
			{
				mipMapSize += mipMap->GetSize();
			}
			m_FillBuffer = Buffer::CreateStagingBuffer(m_PageTable->GetSize() + mipMapSize);
			int bufferOffset = 0;
			commandBuffer.BeginTransfering();
			m_PageTable->Fill(glm::vec4(-1, -1, -1, -1), commandBuffer, m_FillBuffer, bufferOffset);
			for (auto mipMaps : m_PageTableMipMaps)
			{
				bufferOffset += mipMaps->GetSize();
				mipMaps->Fill(glm::vec4(-1, -1,-1,-1), commandBuffer, m_FillBuffer, bufferOffset);
			}
			commandBuffer.EndTransfering();
			commandBuffer.SubmitSingle();

		};


		auto bindResources = [this]()
		{
			auto device = RenderContext::GetDevice();
			iden = glm::scale(iden,  glm::vec3{ 10,10,1 });
			iden = glm::rotate(iden, glm::radians(180.f), glm::vec3{ 0,1,0 }); 
			m_PageTableDescriptorSet = Renderer::Instance()->GetSet<vk::DescriptorSet>(m_PageTableDescCompute, PipelineType::COMPUTE);
			m_PageTableFinalDescriptorSet = Renderer::Instance()->GetSet<vk::DescriptorSet>(m_PageTableDescFinalCompute, PipelineType::COMPUTE);
			m_DescriptorSetWorkingSet = Renderer::Instance()->GetSet<vk::DescriptorSet>(m_WorkingSetDesc, PipelineType::RENDER);
			std::vector<vk::DescriptorImageInfo> imageInfos;

			
			
			for (int i = 0; i < m_PageTableMipMaps.size(); i++)
			{
				vk::DescriptorImageInfo imageDescriptor1;
				imageDescriptor1.imageLayout = vk::ImageLayout::eGeneral;
				imageDescriptor1.imageView = m_PageTableMipMaps[i]->GetImageView();
				imageDescriptor1.sampler = m_PageTableMipMaps[i]->GetSampler();
				imageInfos.push_back(imageDescriptor1);
			}
			vk::DescriptorImageInfo imageDescriptor1;
			imageDescriptor1.imageLayout = vk::ImageLayout::eGeneral;
			imageDescriptor1.imageView = m_PageTable->GetImageView();
			imageDescriptor1.sampler = m_PageTable->GetSampler();
			imageInfos.push_back(imageDescriptor1);

			device->UpdateDescriptorSet(m_PageTableDescriptorSet, 0, imageInfos, vk::DescriptorType::eStorageImage);

			m_DescriptorSetDebug = Renderer::Instance()->GetSet<vk::DescriptorSet>(m_DebugTexturesDesc, PipelineType::RENDER);

			auto bufferPerFrame = Renderer::Instance()->GetSet<std::vector<vk::DescriptorSet>>(m_BaseDesc, PipelineType::RENDER);
			m_DescriptorSets = bufferPerFrame;
			auto framesAmount = RenderContext::GetFrameAmount();
			for (size_t i = 0; i < framesAmount; i++)
			{
				device->UpdateDescriptorSet(m_DescriptorSets[i], 0, 1, *Renderer::Instance()->m_UniformBuffers[i], vk::DescriptorType::eUniformBuffer);
			}

			auto ptr = RenderContext::GetDevice()->GetDevice().mapMemory(m_AddInfo->GetMemory(), 0, sizeof AdditionalData);
			memcpy(ptr, &m_AddData, sizeof AdditionalData);
			RenderContext::GetDevice()->GetDevice().unmapMemory(m_AddInfo->GetMemory());
			device->UpdateDescriptorSet(m_DescriptorSetDebug, 1, 1,
				*m_AddInfo, vk::DescriptorType::eUniformBuffer);

			


			vk::DescriptorImageInfo imageDescriptor;
			imageDescriptor.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			imageDescriptor.imageView = m_WorkingSet->GetImageView();
			imageDescriptor.sampler = m_WorkingSet->GetSampler();

			device->UpdateDescriptorSet(m_DescriptorSetDebug, 0,1, imageDescriptor , vk::DescriptorType::eCombinedImageSampler);
			
			
		};

		

		auto createPipelines = [this]()
		{
			auto m_DescriptorSetLayout = Renderer::Instance()->GetSetLayout(m_BaseDesc, PipelineType::RENDER);
			auto m_DescriptorSetWorkingSetLayout = Renderer::Instance()->GetSetLayout(m_WorkingSetDesc, PipelineType::RENDER);
			auto m_DescriptorSetDebugLayout = Renderer::Instance()->GetSetLayout(m_DebugTexturesDesc, PipelineType::RENDER);
			auto m_DescriptorSetPageTableCompLayout = Renderer::Instance()->GetSetLayout(m_PageTableDescCompute, PipelineType::COMPUTE);
			auto m_DescriptorSetPageTableCompFinalLayout = Renderer::Instance()->GetSetLayout(m_PageTableDescFinalCompute, PipelineType::COMPUTE);
			
			
			



			auto device = RenderContext::GetDevice();

			// render pass
			auto samples = RenderContext::GetDevice()->GetSamples();
			samples = vk::SampleCountFlagBits::e2;
			size_t actualFrameAmount = RenderContext::GetFrameAmount();
		

			m_AttachmentManager.CreateColor("MSAA", m_AttachmentManager, vk::Format::eB8G8R8A8Unorm,
				Application::GetScreenWidth(), Application::GetScreenHeight(), 
				samples, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
				actualFrameAmount);

			m_AttachmentManager.CreateColor("FeedbackBuffer", m_AttachmentManager, vk::Format::eR32G32B32A32Sfloat,
				feedbackSize.x, feedbackSize.y,
				vk::SampleCountFlagBits::e1, vk::ImageUsageFlagBits::eColorAttachment| vk::ImageUsageFlagBits::eSampled| vk::ImageUsageFlagBits::eTransferSrc,
				actualFrameAmount, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			auto stageSize = feedbackSize.x * feedbackSize.y * sizeof(FeedbackRes);
			m_StageBuffers.resize(actualFrameAmount);
			for (int i = 0; i < actualFrameAmount; i++)
			{
				m_StageBuffers[i] = Buffer::CreateStagingBuffer(stageSize);
			}

			m_AttachmentManager.CreateDepthStencil("DepthStencil", m_AttachmentManager,
				Application::GetScreenWidth(), Application::GetScreenHeight(),
				samples, vk::ImageUsageFlagBits::eDepthStencilAttachment,
				1);

			m_AttachmentManager.CreateDepthStencil("FeedbackDepthStencil", m_AttachmentManager,
				Application::GetScreenWidth(), Application::GetScreenHeight(),
				vk::SampleCountFlagBits::e1, vk::ImageUsageFlagBits::eDepthStencilAttachment,
				1);
				

			Renderer::Instance()->CreateSyncObjects();

			UPtr<IExecute> m_FinalRenderPass;
			UPtr<IExecute> m_DebugRenderPass;
			UPtr<IExecute> m_FeedbackRenderPass;
			UPtr<IExecute> m_UpdatePageTablePass;

			{
				std::vector<vk::DescriptorSetLayout> layouts = { m_DescriptorSetPageTableCompLayout->GetLayout()};
				Renderer::Instance()->CompileShader("pageTable.comp", ShaderType::COMPUTE);
				Pipeline::CreateComputePipeline(COMPUTE_PAGE_TABLE_PASS, BASE_SPIRV_OUTPUT +"pageTable.spvCmp", layouts);
			}
			{
				std::vector<vk::DescriptorSetLayout> layouts = { m_DescriptorSetPageTableCompFinalLayout->GetLayout()};
				Renderer::Instance()->CompileShader("pageTableFinal.comp", ShaderType::COMPUTE);
				Pipeline::CreateComputePipeline(COMPUTE_PAGE_TABLE_FINAL_PASS, BASE_SPIRV_OUTPUT +"pageTableFinal.spvCmp", layouts);
			
			}

			uint64_t bufferSize = sizeof(FeedbackRes)*feedbackSize.x* feedbackSize.y;
			m_FeedbackRes.resize(feedbackSize.x* feedbackSize.y);

			
			Func exe = [=](CommandBuffer& cmd, size_t frameIndex)
				{
					ZoneScopedN("Feedback Reading Pass");
					auto device = RenderContext::GetDevice();
					auto color = m_AttachmentManager.GetColor({ "FeedbackBuffer" })[frameIndex];
					auto tracyContext = Renderer::Instance()->GetTracyCtx();
					auto tracyCmd = Renderer::Instance()->GetTracyCmd();
					//tracyCmd.BeginRendering();
					auto transferBuffer = Renderer::Instance()->GetTransferCommandBuffer(frameIndex);
					Fence fence;
					Renderer::Instance()->Reset(fence.GetFence());
					
					transferBuffer.BeginTransfering();
					transferBuffer.CopyImageToBuffer(color, m_StageBuffers[frameIndex]);
					transferBuffer.ChangeImageLayout(m_WorkingSet.get(), m_WorkingSet->GetLayout(), vk::ImageLayout::eTransferDstOptimal,1,workingSetPageAmount);
					transferBuffer.EndTransfering();
					transferBuffer.SubmitSingle(fence.GetFence());
					Renderer::Instance()->Wait(fence.GetFence());

					auto ptr = (float*)device->GetDevice().mapMemory(m_StageBuffers[frameIndex]->GetMemory(), (uint64_t)0, bufferSize);
					//tilesToLoadToPageTable.clear();
					//memcpy(m_FeedbackRes.data(), ptr, bufferSize);
					//std::copy(m_FeedbackRes.begin(), m_FeedbackRes.end(), ptr);

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
					device->GetDevice().unmapMemory(m_StageBuffers[frameIndex]->GetMemory());
					// load tiles

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
					std::vector<PageEntry> tilesWeSee;
					{
						std::vector<std::future<void>> tilesToLoad;
						ZoneScopedN("Reading Feedback Buffer");
						for (auto& feedback : m_FeedbackRes)
						{
							// there is feedback
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
										m_WorkingSetPtr[0] = coords.x ;
										m_WorkingSetPtr[1] = coords.y ;
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
												m_WorkingSetPtr[0] = coords.x ;
												m_WorkingSetPtr[1] = coords.y ;
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

									vk::Offset3D offset{ m_WorkingSetPtr[0],m_WorkingSetPtr[1] ,0};
									int layer = m_WorkingSetPtr[1] * workingSetPageAmountX + m_WorkingSetPtr[0];
									assert(layer < workingSetPageAmount);
									//std::cout << path << std::endl;
									auto future = std::async(std::launch::async, &Image::UpdateRegionWithImage, path, m_WorkingSet, vk::Offset3D{0,0,0}, layer);
									tilesToLoad.push_back(std::move(future));

									float workingSetCoordX = (float)(m_WorkingSetPtr[0]) ;
									float workingSetCoordY = (float)m_WorkingSetPtr[1] ;
									glm::vec2 physCoord = { workingSetCoordX,workingSetCoordY };
									auto mipMap = feedback.mipMap;
									glm::vec2 pageCoord = { feedback.pageX ,feedback.pageY };
									PageEntry page{ mipMap,pageCoord,physCoord };

									m_Cache.Add(page, path);
									tilesWeSee.push_back(page);
								}
								else
								{
									ZoneScopedN("Add exisiting page");
									if (std::find(tilesWeSee.begin(), tilesWeSee.end(), *cachedPage) == tilesWeSee.end())
									{
										tilesWeSee.push_back(*cachedPage);
									}
								}

							}
						}
					}
					

					
					if (tilesWeSee.size() > 0)
					{
						
						ZoneScopedN("Update visible pages");
						
						std::vector<PageEntry> clear(virtualTextureTiles.x* virtualTextureTiles.y);
						m_StorageBuffers->SetData(clear.data());
						auto ptr = (PageEntry*)device->GetDevice().mapMemory(m_StorageBuffers->GetMemory(), (uint64_t)0, tilesWeSee.size()* sizeof(tilesWeSee[0]));
						memcpy(ptr, tilesWeSee.data(), tilesWeSee.size() * sizeof(tilesWeSee[0]));
						device->GetDevice().unmapMemory(m_StorageBuffers->GetMemory());
						device->UpdateDescriptorSet(m_PageTableDescriptorSet,1,1, *m_StorageBuffers,vk::DescriptorType::eStorageBuffer);

					}

						vk::DescriptorImageInfo imageDescriptor;
						imageDescriptor.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
						imageDescriptor.imageView = m_WorkingSet->GetImageView();
						imageDescriptor.sampler = m_WorkingSet->GetSampler();
						
						//device->GetDevice().waitIdle();
						device->UpdateDescriptorSet(m_DescriptorSetWorkingSet, 0, 1, imageDescriptor, vk::DescriptorType::eCombinedImageSampler);
						

						{
							ZoneScopedN("Init page table");
							int bufferOffset = 0;
							transferBuffer.BeginTransfering();
							m_PageTable->Fill(glm::vec4(-1, -1, -1, -1), transferBuffer, m_FillBuffer, bufferOffset);
							for (auto mipMaps : m_PageTableMipMaps)
							{
								bufferOffset += mipMaps->GetSize();
								mipMaps->Fill(glm::vec4(-1, -1, -1, -1), transferBuffer, m_FillBuffer, bufferOffset);
							}
							transferBuffer.ChangeImageLayout(m_WorkingSet.get(), m_WorkingSet->GetLayout(), vk::ImageLayout::eShaderReadOnlyOptimal,1,workingSetPageAmount);
							if (m_PageTable->GetLayout() != vk::ImageLayout::eGeneral)
							{
								transferBuffer.ChangeImageLayout(m_PageTable.get(), m_PageTable->GetLayout(), vk::ImageLayout::eGeneral);
							
							}
							transferBuffer.EndTransfering();
							transferBuffer.SubmitSingle();

						}

						cmd.BeginTransfering();
						{
							ZoneScopedN("Fill page table");
							//update mip levels of page table
							auto currentFrame = frameIndex;
							// we need to insert a barrier to make memory available
							auto pipeline = Renderer::Instance()->GetPipeline(COMPUTE_PAGE_TABLE_PASS);
							vkCmdBindPipeline(cmd.GetCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
							cmd.GetCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeline->GetLayout(), 0, 1, &m_PageTableDescriptorSet, 0, 0);
							float invocations = 8;
							int localSize = 1;
							TracyVkZone(tracyContext, cmd.GetCommandBuffer(), "Update tables");
							vkCmdDispatch(cmd.GetCommandBuffer(), tilesWeSee.size() / localSize, 1, 1);

							std::vector<vk::Image> images{ m_PageTable->GetImage() };
							for (auto mipMaps : m_PageTableMipMaps)
							{
								images.push_back(mipMaps->GetImage());
							}
							
							// sync problem
							device->GetDevice().waitIdle();
							//cmd.ImageBufferBarrier(images, vk::PipelineStageFlagBits::eTopOfPipe,
							//	vk::AccessFlagBits::eNone,
							//	vk::PipelineStageFlagBits::eComputeShader,
							//	vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite);


							
							{
								auto pipelineFinal = Renderer::Instance()->GetPipeline(COMPUTE_PAGE_TABLE_FINAL_PASS);
								TracyVkZone(tracyContext, cmd.GetCommandBuffer(), "merge tables");


								std::vector<vk::DescriptorImageInfo> imageInfos;
								vk::DescriptorImageInfo imageDescriptor1;
								imageDescriptor1.imageLayout = vk::ImageLayout::eGeneral;
								imageDescriptor1.imageView = m_PageTable->GetImageView();
								imageDescriptor1.sampler = m_PageTable->GetSampler();
								device->UpdateDescriptorSet(m_PageTableFinalDescriptorSet, 1, 1, imageDescriptor1, vk::DescriptorType::eStorageImage);
								for (int i = 0; i < m_PageTableMipMaps.size(); i++)
								{
									imageDescriptor1.imageView = m_PageTableMipMaps[i]->GetImageView();
									imageDescriptor1.sampler = m_PageTableMipMaps[i]->GetSampler();
									imageInfos.push_back(imageDescriptor1);
								}

								device->UpdateDescriptorSet(m_PageTableFinalDescriptorSet, 0, imageInfos, vk::DescriptorType::eStorageImage);

								vkCmdBindPipeline(cmd.GetCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, pipelineFinal->GetPipeline());
								cmd.GetCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineFinal->GetLayout(), 0, 1, &m_PageTableFinalDescriptorSet, 0, 0);


								vkCmdDispatch(cmd.GetCommandBuffer(), pageTableWidth, pageTableHeight, 1);
								cmd.ChangeImageLayout(m_PageTable.get(), m_PageTable->GetLayout(), vk::ImageLayout::eShaderReadOnlyOptimal);
								{
									vk::DescriptorImageInfo imageDescriptor1;
									imageDescriptor1.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
									imageDescriptor1.imageView = m_PageTable->GetImageView();
									imageDescriptor1.sampler = m_PageTable->GetSampler();
									device->UpdateDescriptorSet(m_DescriptorSetWorkingSet, 1, 1, imageDescriptor1, vk::DescriptorType::eCombinedImageSampler);
								}

							}
							
						}
						cmd.EndTransfering();
						


				};
			m_UpdatePageTablePass = CreateUPtr<ComputePass>(exe);


			vk::ClearValue clearColor = { std::array<float, 4>{137.f / 255.f, 189.f / 255.f, 199.f / 255.f, 1.0f} };
			vk::Extent2D extent = { (uint32_t)Application::GetScreenWidth(),(uint32_t)Application::GetScreenHeight() };
			vk::Extent2D feedbackExtent = { (uint32_t)feedbackSize.x,(uint32_t)feedbackSize.y };
			vk::ClearValue depthClear;
			uint32_t stencil0 = 3;
			depthClear.depthStencil = vk::ClearDepthStencilValue({ 1.0f, 0 });
			std::vector<vk::ClearValue> clearValues = { {clearColor, depthClear,clearColor} };

		
			// final render pass
			{

				auto samples = RenderContext::GetDevice()->GetSamples();
				RenderPassBuilder builder;
				//Define a general attachment, with its load/store operations
				builder.ColorOutput("MSAA", m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eClear);
				builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetInitialLayout(vk::ImageLayout::eUndefined);
				builder.SetFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
				auto msaaDecs = builder.BuildAttachmentDesc();

				builder.DepthStencilOutput("DepthStencil", m_AttachmentManager, vk::ImageLayout::eDepthStencilAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eClear);
				builder.SetSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetInitialLayout(vk::ImageLayout::eUndefined);
				builder.SetFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				auto depth = builder.BuildAttachmentDesc();

				builder.ResolveOutput("Default", m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eClear);
				builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetInitialLayout(vk::ImageLayout::eUndefined);
				builder.SetFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
				//builder.SetFinalLayout(vk::ImageLayout::ePresentSrcKHR);
				auto resolve = builder.BuildAttachmentDesc();



				


				vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
					vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::AccessFlagBits::eColorAttachmentWrite,
					vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests , vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
				


				builder.AddSubpass({ 0 }, { 1 }, { 2 });
				
				builder.AddSubpassDependency(dependency0);

				
				auto exe = [this](CommandBuffer& commandBuffer, size_t frameIndex)
				{

					ZoneScopedN("Final render pass");
					auto tracyContext = Renderer::Instance()->GetTracyCtx();
					auto tracyCmd = Renderer::Instance()->GetTracyCmd();
					//tracyCmd.BeginRendering();
					auto vkCommandBuffer = commandBuffer.GetCommandBuffer();
					auto camera = GetCamera();
					camera->UpdateProj(Application::GetScreenWidth(),
						Application::GetScreenHeight(), glm::radians(45.f));
					Renderer::Instance()->UpdateUniformBuffer(camera->GetProj(), *camera);
					TracyVkZone(tracyContext, vkCommandBuffer, "Final render pass");

					Renderer::Instance()->BeginBatch(); 
					auto pipeline = Renderer::Instance()->GetPipeline(RENDER_BASIC_PASS);
					vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 0, m_DescriptorSets[frameIndex], nullptr); 
					vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 1, m_DescriptorSetWorkingSet, nullptr);
					vkCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline()); 
					vk::Viewport viewport; 
					viewport.x = 0.0f; 
					viewport.y = 0.0f; 
					viewport.minDepth = 0; 
					viewport.maxDepth = 1; 
					viewport.width = Application::GetScreenWidth(); 
					viewport.height = Application::GetScreenHeight(); 
					vk::Rect2D scissors; 
					scissors.offset = vk::Offset2D{ (uint32_t)0,(uint32_t)0 }; 
					scissors.extent = vk::Extent2D{ (uint32_t)viewport.width,(uint32_t)viewport.height }; 
					vkCommandBuffer.setViewport(0, 1, &viewport);
					vkCommandBuffer.setScissor(0, 1, &scissors); 
					
					Renderer::Instance()->Draw(m_Plane, iden);
					Renderer::Instance()->DrawBatch(vkCommandBuffer);
					//Renderer::Instance()->DrawQuadScreen(vkCommandBuffer);
				};

				m_FinalRenderPass = builder.Build(RENDER_BASIC_PASS, m_AttachmentManager, actualFrameAmount, extent, clearValues, exe);

				m_Plane.Pos = { 0,0,0 };
				//GetCamera()->LookAt(m_Plane.Pos);
			}
			std::vector<vk::VertexInputBindingDescription> bindings
			{
				VertexBindingDescription(0,sizeof(Vertex),vk::VertexInputRate::eVertex),
			};


			std::vector<vk::VertexInputAttributeDescription> attributeDescriptions =
			{
				VertexInputAttributeDescription(0,0,vk::Format::eR32G32B32Sfloat,offsetof(Vertex, Position)),
				VertexInputAttributeDescription(0,1,vk::Format::eR32G32Sfloat,offsetof(Vertex, UV)),
				VertexInputAttributeDescription(0,2,vk::Format::eR32G32B32A32Sfloat,offsetof(Vertex, Color)),
				VertexInputAttributeDescription(0,3,vk::Format::eR32Sfloat,offsetof(Vertex, textureID)),
			};

			{
				auto pipelineLayouts = std::vector<vk::DescriptorSetLayout>{ m_DescriptorSetLayout->GetLayout(),m_DescriptorSetWorkingSetLayout->GetLayout()};
				PipelineBuilder builder;
				builder.SetDevice(device->GetDevice());
				builder.SetSamples(samples);
				builder.AddDescriptorLayouts(pipelineLayouts);
				builder.AddAttributeDescription(attributeDescriptions);
				builder.AddBindingDescription(bindings);
				builder.SetPolygoneMode(Renderer::Instance()->GetPolygonMode());
				builder.SetTopology(vk::PrimitiveTopology::eTriangleList);
				Renderer::Instance()->CompileShader("feedback.spvV", ShaderType::VERTEX);
				Renderer::Instance()->CompileShader("render_working_set.spvF", ShaderType::FRAGMENT);
				builder.AddShader(BASE_SPIRV_OUTPUT + "feedback.spvV", vk::ShaderStageFlagBits::eVertex);
				builder.AddShader(BASE_SPIRV_OUTPUT + "render_working_set.spvF", vk::ShaderStageFlagBits::eFragment);
				builder.SetSubpassAmount(0);
				builder.AddExtent(extent);
				builder.AddImageFormat(vk::Format::eB8G8R8A8Unorm);
				builder.EnableStencilTest(false);
				builder.SetDepthTest(true);
				builder.WriteToDepthBuffer(true);
				builder.SetRenderPass(static_cast<RenderPass*>(m_FinalRenderPass.get())->GetRaw());
				builder.SetStencilRefNumber(2);
				builder.StencilTestOp(vk::CompareOp::eAlways, vk::StencilOp::eReplace, vk::StencilOp::eReplace, vk::StencilOp::eReplace);
				builder.SetMasks(0xff, 0xff);
				builder.SetPolygoneMode(vk::PolygonMode::eFill);
				builder.Build(RENDER_BASIC_PASS);
			}
			// debug render pass
			{

				auto samples = RenderContext::GetDevice()->GetSamples();
				RenderPassBuilder builder;
				//Define a general attachment, with its load/store operations

				builder.ColorOutput("Default", m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eLoad);
				builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetInitialLayout(vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetFinalLayout(vk::ImageLayout::ePresentSrcKHR);
				auto resolve = builder.BuildAttachmentDesc();






				vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
					vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::AccessFlagBits::eColorAttachmentWrite,
					vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);



				builder.AddSubpass({ 0 }, { 1 }, { 2 });

				builder.AddSubpassDependency(dependency0);

				auto exe = [this ](CommandBuffer& commandBuffer, size_t frameIndex)
					{
						ZoneScopedN("debug render pass");
						auto tracyContext = Renderer::Instance()->GetTracyCtx();
						auto tracyCmd = Renderer::Instance()->GetTracyCmd();
						//tracyCmd.BeginRendering();
						auto vkCommandBuffer = commandBuffer.GetCommandBuffer();

						TracyVkZone(tracyContext, vkCommandBuffer, "Debug render pass ");
						
						auto device = RenderContext::GetDevice();
						
						if (m_WorkingSet->GetLayout() != vk::ImageLayout::eShaderReadOnlyOptimal)
						{
							auto transferCommandBuffer = Renderer::Instance()->GetTransferCommandBuffer(frameIndex);
							auto cmdBuffer = transferCommandBuffer.BeginTransfering();
							transferCommandBuffer.ChangeImageLayout(m_WorkingSet.get(), m_WorkingSet->GetLayout(), vk::ImageLayout::eShaderReadOnlyOptimal,1, workingSetPageAmount);
							transferCommandBuffer.EndTransfering();
							transferCommandBuffer.SubmitSingle();
						}
						Renderer::Instance()->BeginBatch();
						auto pipeline = Renderer::Instance()->GetPipeline(RENDER_DEBUG_PASS);
						vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 0, m_DescriptorSets[frameIndex], nullptr);
						vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 1, m_DescriptorSetDebug, nullptr); 
						vkCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());
						vk::Viewport viewport;
						viewport.x = 0.0f;
						viewport.y = 0.0f;
						viewport.minDepth = 0;
						viewport.maxDepth = 1;
						viewport.width = Application::GetScreenWidth();
						viewport.height = Application::GetScreenHeight();
						vk::Rect2D scissors;
						scissors.offset = vk::Offset2D{ (uint32_t)0,(uint32_t)0 };
						scissors.extent = vk::Extent2D{ (uint32_t)viewport.width,(uint32_t)viewport.height };
						vkCommandBuffer.setViewport(0, 1, &viewport);
						vkCommandBuffer.setScissor(0, 1, &scissors);
						Renderer::Instance()->DrawQuadScreen(vkCommandBuffer);
						//tracyCmd.EndRendering();
					};

				m_DebugRenderPass = builder.Build(RENDER_DEBUG_PASS, m_AttachmentManager, actualFrameAmount, extent, clearValues, exe);
			}

			{
				auto pipelineLayouts = std::vector<vk::DescriptorSetLayout>{ m_DescriptorSetLayout->GetLayout(),m_DescriptorSetDebugLayout->GetLayout() };
				PipelineBuilder builder;
				builder.SetDevice(device->GetDevice());
				builder.SetSamples(vk::SampleCountFlagBits::e1);
				builder.AddDescriptorLayouts(pipelineLayouts);
				builder.AddAttributeDescription(std::vector<vk::VertexInputAttributeDescription>{});
				builder.AddBindingDescription(std::vector<vk::VertexInputBindingDescription>{});
				builder.SetPolygoneMode(Renderer::Instance()->GetPolygonMode());
				builder.SetTopology(vk::PrimitiveTopology::eTriangleList);
				Renderer::Instance()->CompileShader("debug.spvV", ShaderType::VERTEX);
				Renderer::Instance()->CompileShader("debug.spvF", ShaderType::FRAGMENT);
				builder.AddShader(BASE_SPIRV_OUTPUT + "debug.spvV", vk::ShaderStageFlagBits::eVertex);
				builder.AddShader(BASE_SPIRV_OUTPUT + "debug.spvF", vk::ShaderStageFlagBits::eFragment);
				builder.SetSubpassAmount(0);
				builder.AddExtent(extent);
				builder.AddImageFormat(vk::Format::eB8G8R8A8Unorm);
				builder.EnableStencilTest(false);
				builder.SetDepthTest(false);
				builder.WriteToDepthBuffer(false);
				builder.SetRenderPass(static_cast<RenderPass*>(m_DebugRenderPass.get())->GetRaw());
				builder.SetStencilRefNumber(2);
				builder.StencilTestOp(vk::CompareOp::eAlways, vk::StencilOp::eReplace, vk::StencilOp::eReplace, vk::StencilOp::eReplace);
				builder.SetMasks(0xff, 0xff);
				builder.SetPolygoneMode(vk::PolygonMode::eFill);
				builder.Build(RENDER_DEBUG_PASS);
			}

			// feedbackPass
			{

				auto samples = RenderContext::GetDevice()->GetSamples();
				RenderPassBuilder builder;
				builder.ColorOutput("FeedbackBuffer", m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eClear);
				builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetInitialLayout(vk::ImageLayout::eUndefined);
				builder.SetFinalLayout(vk::ImageLayout::eTransferSrcOptimal);
				builder.BuildAttachmentDesc();

				builder.DepthStencilOutput("FeedbackDepthStencil", m_AttachmentManager, vk::ImageLayout::eDepthStencilAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eClear);
				builder.SetSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetInitialLayout(vk::ImageLayout::eUndefined);
				builder.SetFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				auto depth = builder.BuildAttachmentDesc();
				vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite,
					vk::PipelineStageFlagBits::eColorAttachmentOutput , vk::AccessFlagBits::eColorAttachmentWrite);
				builder.AddSubpass({ 0 }, { 1 }, { 2 });
				builder.AddSubpassDependency(dependency0);
				Func exe1 = [this](CommandBuffer& commandBuffer, size_t frameIndex)
					{
						ZoneScopedN("feedback pass ");
						auto camera = GetCamera();
						camera->UpdateProj(feedbackSize.x, feedbackSize.y,glm::radians(55.f));
						Renderer::Instance()->UpdateUniformBuffer(camera->GetProj(), *camera);
						auto vkCommandBuffer = commandBuffer.GetCommandBuffer();
						auto tracyContext = Renderer::Instance()->GetTracyCtx();
						TracyVkZone(tracyContext, vkCommandBuffer, "feedback pass ");
						//tracyCmd.EndRendering();
						Renderer::Instance()->BeginBatch();
						auto pipeline = Renderer::Instance()->GetPipeline(FEEDBACK_RENDER_PASS);
						vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 0, m_DescriptorSets[frameIndex], nullptr);
						//vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 1, m_DescriptorSetTex, nullptr);
						vkCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());
						vk::Viewport viewport;
						viewport.x = 0.0f;
						viewport.y = 0.0f;
						viewport.minDepth = 0;
						viewport.maxDepth = 1;
						viewport.width = feedbackSize.x;
						viewport.height = feedbackSize.y;
						vk::Rect2D scissors;
						scissors.offset = vk::Offset2D{ (uint32_t)0,(uint32_t)0 };
						scissors.extent = vk::Extent2D{ (uint32_t)viewport.width,(uint32_t)viewport.height };
						vkCommandBuffer.setViewport(0, 1, &viewport);
						vkCommandBuffer.setScissor(0, 1, &scissors);
						Renderer::Instance()->Draw(m_Plane, iden);
						Renderer::Instance()->DrawBatch(vkCommandBuffer);
						
					};
				m_FeedbackRenderPass = builder.Build(FEEDBACK_RENDER_PASS, m_AttachmentManager, actualFrameAmount, feedbackExtent, { {std::array<float, 4>{137.f / 255.f, 189.f / 255.f, 199.f / 255.f, 0.0f} }, depthClear }, exe1);
			}

			{
				auto pipelineLayouts = std::vector<vk::DescriptorSetLayout>{ m_DescriptorSetLayout->GetLayout()/*,m_DescriptorSetLayoutTex->GetLayout()*/ };
				PipelineBuilder builder;
				builder.SetDevice(device->GetDevice());
				builder.SetSamples(vk::SampleCountFlagBits::e1);
				builder.AddDescriptorLayouts(pipelineLayouts);
				builder.AddAttributeDescription(attributeDescriptions);
				builder.AddBindingDescription(bindings);
				builder.SetPolygoneMode(Renderer::Instance()->GetPolygonMode());
				builder.SetTopology(vk::PrimitiveTopology::eTriangleList);
				Renderer::Instance()->CompileShader("feedback.spvV", ShaderType::VERTEX);
				Renderer::Instance()->CompileShader("feedback.spvF", ShaderType::FRAGMENT);
				builder.AddShader(BASE_SPIRV_OUTPUT + "feedback.spvV", vk::ShaderStageFlagBits::eVertex);
				builder.AddShader(BASE_SPIRV_OUTPUT + "feedback.spvF", vk::ShaderStageFlagBits::eFragment);
				builder.SetSubpassAmount(0);
				builder.AddExtent(feedbackExtent);
				builder.AddImageFormat(vk::Format::eR8G8B8A8Uint);
				builder.EnableStencilTest(false);
				builder.SetDepthTest(true);
				builder.EnableBlend(false);
				builder.WriteToDepthBuffer(true);
				builder.SetRenderPass(static_cast<RenderPass*>(m_FeedbackRenderPass.get())->GetRaw());
				builder.SetPolygoneMode(vk::PolygonMode::eFill);
				builder.Build(FEEDBACK_RENDER_PASS);
			}
			
		

		
			// ImGui
			RenderPassBuilder builder;
			builder.ColorOutput("Default", m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
			builder.SetLoadOp(vk::AttachmentLoadOp::eLoad);
			builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
			builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
			builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
			builder.SetInitialLayout(vk::ImageLayout::eColorAttachmentOptimal);
			builder.SetFinalLayout(vk::ImageLayout::ePresentSrcKHR);
			auto defau = builder.BuildAttachmentDesc();

			builder.AddSubpass({ 0 }, { 0 }, { 0 });

			vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
				vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite,
				vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);
			builder.AddSubpassDependency(dependency0);

			UPtr<IExecute> m_ImGuiRenderPass = builder.Build(IMGUI_RENDER_PASS, m_AttachmentManager,
				RenderContext::GetFrameAmount(),
				extent, clearValues, [this](CommandBuffer& commandBuffer, size_t frameIndex)
				{
					ImGui_ImplVulkan_NewFrame();
					ImGui_ImplGlfw_NewFrame();
					ImGui::NewFrame();
					bool show_demo_window = true;
					bool show_another_window = false;
					ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

					// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
					if (show_demo_window)
						ImGui::ShowDemoWindow(&show_demo_window);



					//ImGui::Begin("Surface paramentrs", &show_another_window);
					//ImGui::End();


					// Rendering
					ImGui::Render();
					ImDrawData* draw_data = ImGui::GetDrawData();
					const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
					if (!is_minimized)
					{
						// Record dear imgui primitives into command buffer
						ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer.GetCommandBuffer());

					}
				});

			

			//auto device = RenderContext::GetDevice();
			auto surface = RenderContext::GetSurface();
			auto instance = RenderContext::GetInstance();
			{
				VkDescriptorPoolSize pool_sizes[] =
				{
					{ VK_DESCRIPTOR_TYPE_SAMPLER, 10 },
					{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 },
					{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 10 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 10 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 10 },
					{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 10 }
				};
				VkDescriptorPoolCreateInfo pool_info = {};
				pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
				pool_info.maxSets = 10 * IM_ARRAYSIZE(pool_sizes);
				pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
				pool_info.pPoolSizes = pool_sizes;
				vkCreateDescriptorPool(device->GetDevice(), &pool_info, VK_NULL_HANDLE, &g_DescriptorPool);
			}




			ImGui::CreateContext();
			// Setup Dear ImGui style
			//ImGui::StyleColorsDark();
			ImGui::StyleColorsLight();

			// Setup Platform/Renderer backends
			bool result = ImGui_ImplGlfw_InitForVulkan(Application::GetWindow()->GetRaw(), true);
			//this initializes imgui for Vulkan
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = instance->GetInstance();
			init_info.PhysicalDevice = device->GetDevicePhys();
			init_info.Device = device->GetDevice();
			init_info.Queue = device->GetGraphicsQueue();
			init_info.DescriptorPool = g_DescriptorPool;
			init_info.MinImageCount = 2;
			init_info.ImageCount = RenderContext::GetFrameAmount();
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			result = ImGui_ImplVulkan_Init(&init_info, static_cast<RenderPass*>(m_ImGuiRenderPass.get())->GetRaw());
			auto commandBuffer = Renderer::Instance()->GetTransferCommandBuffer(0);
			commandBuffer.BeginTransfering();
			result = ImGui_ImplVulkan_CreateFontsTexture((VkCommandBuffer)commandBuffer.GetCommandBuffer());
			commandBuffer.EndTransfering();
			commandBuffer.SubmitSingle();
			ImGui_ImplVulkan_DestroyFontUploadObjects();


			auto graph = CreateUPtr<RenderPassGraph>();
			graph->AddExec(std::move(m_FeedbackRenderPass));
			graph->AddExec(std::move(m_UpdatePageTablePass));
			graph->AddExec(std::move(m_FinalRenderPass));
			graph->AddExec(std::move(m_DebugRenderPass));
			//graph->AddExec(std::move(m_ImGuiRenderPass));

			Renderer::Instance()->AddRenderGraph("",std::move(graph));




					
		};

		auto cleanup = [this]()
		{



				auto device = RenderContext::GetDevice()->GetDevice();
				device.waitIdle();
				m_AddInfo.reset();
				m_StorageBuffers.reset();
				m_FillBuffer.reset();
				m_PageTable.reset();
				m_WorkingSet.reset();
				m_StageBuffers.clear();
				m_PageTableMipMaps.clear();
				m_AttachmentManager.Destroy();
				CleanUpImGui();
				Renderer::Instance()->Shutdown();

			};

		callables.bindingsInit = bindingsInit;
		callables.createResources = createResources;
		callables.bindResources = bindResources;
		callables.createPipelines = createPipelines;
		callables.cleanUp= cleanup;


		Renderer::Instance()->SetCallables(callables);
		Renderer::Instance()->UserInit();
		
	}




	~ExampleApplication()
	{
		callables.cleanUp();
	}
	
	
	void UpdateTexture(size_t frameIndex)
	{
		
	}


	void LoadFont(std::string_view str, SPtr<Image> fontAtlas)
	{


		FT_Library ft;
		if (FT_Init_FreeType(&ft) != 0)
		{
			Log::GetLog()->error("ERROR::FREETYPE: Could not init FreeType Library" );
		}

		FT_Face face;
		if (FT_New_Face(ft, str.data(), 0, &face) != 0 )
		{
			Log::GetLog()->error("ERROR::FREETYPE: Failed to load font {0}", str );
		}
		auto error = FT_Set_Pixel_Sizes(face, 0, 40);



		int width = 0;
		int maxWidthTexture = 0;
		int maxGlyphHeight = 0;
		int maxGlyphWidth = 0;
		int characterCount = 0;
		int rowNumber = 0;
		const int charactersPerRow = 10;
		const int padding = 15;
		for (unsigned char c = 0; c < 128; c++)
		{
			// load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER ) != 0)
			{
				Log::GetLog()->error("RROR::FREETYTPE: Failed to load Glyph {}", c);
			}
			// generate texture

			// calculate image size
			width += face->glyph->bitmap.width;
			maxGlyphHeight = std::max((unsigned)maxGlyphHeight, face->glyph->bitmap.rows);
			//maxGlyphWidth = std::max((unsigned)maxGlyphWidth, face->glyph->bitmap.width);
			characterCount++;
			if (characterCount >= charactersPerRow )
			{
				rowNumber++;
				maxWidthTexture = std::max(maxWidthTexture, width);
				characterCount = width = 0;
			}




		}

		auto maxHeight = maxGlyphHeight * (rowNumber+1);
		maxWidthTexture  = (maxWidthTexture + charactersPerRow * padding) + 10;
		//maxHeight = 590;
		auto usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
		fontAtlas = Image::CreateEmptyImage(maxWidthTexture, maxHeight, vk::Format::eR8Unorm, usage);

		auto computeCommandBuffer = Renderer::Instance()->GetComputeCommandBuffer(0);

		computeCommandBuffer.BeginTransfering();
		computeCommandBuffer.ChangeImageLayout(fontAtlas.get(), vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferDstOptimal);
		computeCommandBuffer.EndTransfering();
		computeCommandBuffer.SubmitSingle();
		
		int character_count = 0;
		int increment_x = padding;
		int increment_y = 0;

		

		for (unsigned char c = 0; c < 128; c++)
		{
			// load character glyph 
			auto error = FT_Load_Char(face, c, FT_LOAD_RENDER);
			if (error)
			{
				std::cout << "ERROR: failed to load character" << c << "\n";
				continue;
			}
			auto imageSize = face->glyph->bitmap.width * face->glyph->bitmap.rows;
		
		// space character
			if (imageSize == 0)
			{
				continue;
			}

			BufferInputChunk inputBuffer;
			inputBuffer.size = imageSize;
			inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			inputBuffer.usage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;

			//auto buffer = Buffer::CreateStagingBuffer(imageSize);
			auto buffer = CreateUPtr<Buffer>(inputBuffer);
			auto ptr = RenderContext::GetDevice()->GetDevice().mapMemory(buffer->GetMemory(), 0, imageSize);
			memcpy(ptr, face->glyph->bitmap.buffer, imageSize);
			RenderContext::GetDevice()->GetDevice().unmapMemory(buffer->GetMemory());
			//glTexSubImage2D(GL_TEXTURE_2D, 0, increment_x, increment_y, glyph->bitmap.width, glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer); 
			computeCommandBuffer.BeginTransfering();
			computeCommandBuffer.CopyBufferToImage(*buffer.get(), fontAtlas->GetImage(), face->glyph->bitmap.width, face->glyph->bitmap.rows, 0,{ increment_x,increment_y,0 });
			computeCommandBuffer.EndTransfering();
			computeCommandBuffer.SubmitSingle();


			float texCoordLeft = increment_x;
			float texCoordRight = increment_x+ face->glyph->bitmap.width;
			float texCoordTop = increment_y+ face->glyph->bitmap.rows;
			float texCoordBottom = increment_y;

			Character character;
			character.minUv = {texCoordLeft/ maxWidthTexture,texCoordBottom/ maxHeight };
			character.maxUv = { texCoordRight / maxWidthTexture,texCoordTop / maxHeight };
			character.Advance = face->glyph->advance.x;
			character.Size = { face->glyph->bitmap.width ,face->glyph->bitmap.rows };
			character.Bearing = { face->glyph->bitmap_left,face->glyph->bitmap_top };
			Character::lineSpacing = face->height;
			Characters.insert(std::make_pair(c, character));
			
			increment_x += face->glyph->bitmap.width+ padding;
			character_count++;
			if (character_count >= charactersPerRow)
			{
				increment_y += maxGlyphHeight;
				increment_x = padding;
				character_count = width = 0;
			}
		}

		FT_Done_Face(face);
		FT_Done_FreeType(ft);
		computeCommandBuffer.BeginTransfering();
		computeCommandBuffer.ChangeImageLayout(fontAtlas.get(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
		computeCommandBuffer.EndTransfering();
		computeCommandBuffer.SubmitSingle();


	}

	void PreRender(Camera& camera) override
	{
		static size_t currentFrame = 0;

		const auto scale = glm::vec3(glm::vec3(400, 512, 0));
		if (Input::IsKeyTyped(VS_KEY_I))
		{
			m_AddData.Debug  = !m_AddData.Debug;
			auto ptr = RenderContext::GetDevice()->GetDevice().mapMemory(m_AddInfo->GetMemory(), 0, sizeof (AdditionalData));
			memcpy(ptr, &m_AddData, sizeof (AdditionalData));
			RenderContext::GetDevice()->GetDevice().unmapMemory(m_AddInfo->GetMemory());
			RenderContext::GetDevice()->GetDevice().waitIdle();
			RenderContext::GetDevice()->UpdateDescriptorSet(m_DescriptorSetDebug, 1, 1,
				*m_AddInfo, vk::DescriptorType::eUniformBuffer);
		}
		
		auto mousePos = Input::GetMousePos();
		glm::vec2 currentPos = { std::get<0>(mousePos),std::get<1>(mousePos) };
		static glm::vec2 prevPos;
		//if (Input::IsMousePressed(0))
		//{
		//	auto direction = currentPos - prevPos;
		//
		//
		//}
		//else
		//{
		//	prevPos = currentPos;
		//}

		float speed = 50;

		/*if (Input::IsKeyPressed(VS_KEY_W))
		{
			m_SphereRot.x += 0.01f * speed;
		}

		if (Input::IsKeyPressed(VS_KEY_S))
		{
			m_SphereRot.x-= 0.01f * speed;
		}

		if (Input::IsKeyPressed(VS_KEY_A))
		{

			m_SphereRot.y-= 0.01f * speed;
		}

		if (Input::IsKeyPressed(VS_KEY_D))
		{
			m_SphereRot.y+= 0.01f * speed;
		}
		m_Sphere.Rot = m_SphereRot;*/
		
		
		
		currentFrame++;
		currentFrame %= RenderContext::GetFrameAmount();
		
	}



private:
	struct AdditionalData
	{
		bool Debug = true;
	};
	SPtr<Buffer> m_FillBuffer;
	AdditionalData m_AddData;
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
	UPtr<Buffer> m_StorageBuffers;
	std::vector<SPtr<Buffer>> m_StageBuffers;
	vk::DescriptorSet m_DescriptorSetDebug;
	vk::DescriptorSet m_DescriptorSetWorkingSet;
	glm::vec2 m_Follow;
	SPtr<Image> m_Image;
	int m_WorkingSetPtr[2] = {-1,0};
	bool m_Overload = false;
	SPtr<Image> m_WorkingSet;
	SPtr<Image> m_PageTable;
	std::vector<SPtr<Image>> m_PageTableMipMaps;


	std::vector<glm::vec2> m_ClickPoints;

	
	bool isClicked = false;
	bool isDragged = false;
	struct FeedbackRes
	{
		float pageX, pageY, mipMap, isValid;
	};
	std::vector<FeedbackRes> m_FeedbackRes;
	Callables callables;
	AttachmentManager m_AttachmentManager;
	Quad m_Plane;
	glm::mat4 iden{ 1 };
	glm::vec3  m_SphereRot = {0,0,0};
	Cache<workingSetPageAmount> m_Cache;
};


Voidstar::Application* Voidstar::CreateApplication()
{
	auto str = std::string("Example");
	// 120 -> 1920 * 1080
	// 110
	const int res = 120;
	return new ExampleApplication(str, std::min(16 * res,1920), std::min(9 * res, 1061));
}
int main()
{
	return Main();
}
