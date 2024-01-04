

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
const float PageRenderWidth = 400.f;
const float PageRenderHeight = 512.f;
const int PageAmount = 4;
std::string_view BASIC_RENDER_PASS = "Basic";
std::string_view IMGUI_RENDER_PASS = "ImGui";
std::string_view CLIP_RENDER_PASS = "Clip";
std::string_view NEW_PAGE_RENDER_PASS = "NewPage";
std::string_view NEW_PAGE_RIGHT_RENDER_PASS = "NewPageRight";
std::string_view PAGE_RENDER_PASS = "Page";


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


	ExampleApplication(std::string appName, size_t screenWidth, size_t screenHeight) : Voidstar::Application(appName, screenWidth, screenHeight)
	{
		
		Settings params{3};
		m_ClickPoints.resize(MAX_POINTS, glm::vec2(-1, -1));
		auto renderInit = [&params]()
		{

		};

		auto bindingsInit = [this]()
		{
			

			auto binderRender = Binder<RENDER>();
			m_BaseDesc = binderRender.BeginBind(3);
			binderRender.Bind(0, 1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eTessellationControl
				| vk::ShaderStageFlagBits::eTessellationEvaluation | vk::ShaderStageFlagBits::eFragment);
			binderRender.Bind(1, 1, vk::DescriptorType::eUniformBuffer,  vk::ShaderStageFlagBits::eFragment);
			
			m_TexDesc = binderRender.BeginBind();
			binderRender.Bind(0, PageAmount, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);

			m_TexAtlas = binderRender.BeginBind();
			binderRender.Bind(0, 1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
			
			auto binderCompute = Binder<COMPUTE>();
			m_Compute = binderCompute.BeginBind();
			binderCompute.Bind(0, 1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eCompute);
			binderCompute.Bind(1, 1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
			binderCompute.Bind(2, 1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);

		};

		auto createResources= [this]()
		{
			{
				BufferInputChunk info;
				info.size = sizeof(glm::vec2) * MAX_POINTS;
				info.usage = vk::BufferUsageFlagBits::eStorageBuffer |
					vk::BufferUsageFlagBits::eTransferDst;
				info.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible |
					vk::MemoryPropertyFlagBits::eHostCoherent;
				m_ShaderStorageBuffer = CreateUPtr<Buffer>(info);
			}
			m_MouseInfo.resize(RenderContext::GetFrameAmount());
			for (int i = 0; i < RenderContext::GetFrameAmount(); i++)
			{
				BufferInputChunk inputBuffer;
				inputBuffer.size = sizeof glm::vec2;
				inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
				inputBuffer.usage = vk::BufferUsageFlagBits::eUniformBuffer;
				m_MouseInfo[i] = CreateUPtr<Buffer>(inputBuffer);
			}
			const float noiseTextureWidth = 256.f;
			const float noiseTextureHeight = 256.f;
			auto usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
			m_ImageSelected = Image::CreateEmptyImage(noiseTextureWidth, noiseTextureHeight, vk::Format::eR8G8B8A8Snorm, usage);
			m_Image = Image::CreateImage(BASE_RES_PATH + "dos_2_noise.png");
		};


		auto bindResources = [this]()
		{
			vk::DescriptorImageInfo imageDescriptor;
			imageDescriptor.imageLayout = vk::ImageLayout::eGeneral;
			imageDescriptor.imageView = m_ImageSelected->GetImageView();
			imageDescriptor.sampler = m_ImageSelected->GetSampler();


			m_DescriptorSetSelected = Renderer::Instance()->GetSet<vk::DescriptorSet>(m_Compute, PipelineType::COMPUTE);
			m_DescriptorSetTex = Renderer::Instance()->GetSet<vk::DescriptorSet>(m_TexDesc, PipelineType::RENDER);
			m_DescriptorSetFont = Renderer::Instance()->GetSet<vk::DescriptorSet>(m_TexAtlas, PipelineType::RENDER);

			auto device = RenderContext::GetDevice();

			auto bufferPerFrame = Renderer::Instance()->GetSet<std::vector<vk::DescriptorSet>>(m_BaseDesc, PipelineType::RENDER);
			m_DescriptorSets = bufferPerFrame;
			auto framesAmount = RenderContext::GetFrameAmount();
			for (size_t i = 0; i < framesAmount; i++)
			{
				device->UpdateDescriptorSet(m_DescriptorSets[i], 0, 1, *Renderer::Instance()->m_UniformBuffers[i], vk::DescriptorType::eUniformBuffer);
			}

			vk::DescriptorImageInfo imageDescriptor1;
			imageDescriptor1.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			imageDescriptor1.imageView = m_Image->GetImageView();
			imageDescriptor1.sampler = m_Image->GetSampler();
			device->UpdateDescriptorSet(m_DescriptorSetSelected, 0, 1, imageDescriptor, vk::DescriptorType::eStorageImage);
			device->UpdateDescriptorSet(m_DescriptorSetSelected, 2, 1, *m_Image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eCombinedImageSampler);


			UpdateBuffer(*device, m_DescriptorSetSelected, m_ClickPoints.data(), *m_ShaderStorageBuffer, MAX_POINTS * sizeof(glm::vec2));
			LoadFont(BASE_RES_PATH + "fonts/CrimsonText-Regular.ttf");
			device->UpdateDescriptorSet(m_DescriptorSetFont, 0, 1, *m_FontAtlas, vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eCombinedImageSampler);
		};

		auto createPipelines = [this]()
		{
			auto m_DescriptorSetLayout = Renderer::Instance()->GetSetLayout(0, PipelineType::RENDER);
			auto m_DescriptorSetLayoutFont = Renderer::Instance()->GetSetLayout(m_TexAtlas, PipelineType::RENDER);
			auto m_DescriptorSetLayoutTex = Renderer::Instance()->GetSetLayout(1, PipelineType::RENDER);
			auto m_DescriptorSetLayoutSelected = Renderer::Instance()->GetSetLayout(0, PipelineType::COMPUTE);
			
			
			
			std::vector< vk::DescriptorSetLayout>layouts = { m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutSelected->GetLayout() };



			auto device = RenderContext::GetDevice();

			// render pass
			auto samples = RenderContext::GetDevice()->GetSamples();
			samples = vk::SampleCountFlagBits::e2;
			size_t actualFrameAmount = RenderContext::GetFrameAmount();
		

			m_AttachmentManager.CreateColor("MSAA", m_AttachmentManager, vk::Format::eB8G8R8A8Unorm,
				Application::GetScreenWidth(), Application::GetScreenHeight(), 
				samples, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
				actualFrameAmount);

			m_AttachmentManager.CreateDepthStencil("DepthStencil", m_AttachmentManager,
				Application::GetScreenWidth(), Application::GetScreenHeight(),
				samples, vk::ImageUsageFlagBits::eDepthStencilAttachment,
				actualFrameAmount);

			
				

			Renderer::Instance()->CreateSyncObjects();

			UPtr<RenderPass> m_RenderPass;
			
			UPtr<RenderPass> m_ClipRenderPass;
			UPtr<RenderPass> m_NewPageRenderPass;
			UPtr<RenderPass> m_NewPageRenderPassRight;
			UPtr<Pipeline> m_PagePipeline;
			UPtr<Pipeline> m_GraphicsPipeline;
			UPtr<Pipeline> m_ClipPipeline;
			UPtr<Pipeline> m_NewPagePipeline;
			UPtr<Pipeline> m_NewPagePipelineRight;

			vk::ClearValue clearColor = { std::array<float, 4>{137.f / 255.f, 189.f / 255.f, 199.f / 255.f, 1.0f} };
			vk::Extent2D extent = { (uint32_t)Application::GetScreenWidth(),(uint32_t)Application::GetScreenHeight() };
			vk::ClearValue depthClear;
			uint32_t stencil0 = 3;
			depthClear.depthStencil = vk::ClearDepthStencilValue({ 1.0f, stencil0 });
			std::vector<vk::ClearValue> clearValues = { {clearColor, depthClear,clearColor} };

		
			
			{


			
				
			
			
				auto samples = RenderContext::GetDevice()->GetSamples();
				
			
			
				AttachmentSpec page;
				page.Specs.width = PageRenderWidth;
				page.Specs.height = PageRenderHeight;
				page.Specs.usage = vk::ImageUsageFlagBits::eColorAttachment
					| vk::ImageUsageFlagBits::eTransferDst | 
					vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
				page.Specs.format = vk::Format::eB8G8R8A8Unorm;
				page.Specs.samples = vk::SampleCountFlagBits::e1;
				page.Specs.minFilter= vk::Filter::eLinear;
				page.Specs.magFilter= vk::Filter::eLinear;
				page.Specs.tiling= vk::ImageTiling::eOptimal;
				page.Specs.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
				page.Amount = actualFrameAmount;
				page.Samples = vk::SampleCountFlagBits::e1;
			

			
			

				

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
				builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eClear);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetInitialLayout(vk::ImageLayout::eUndefined);
				builder.SetFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				auto depth = builder.BuildAttachmentDesc();



				////Define a general attachment, with its load/store operations
				//vk::AttachmentDescription colorAttachmentResolve =
				//	AttachmentDescription(swapchainFormat, vk::SampleCountFlagBits::e1,
				//		vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
				//		vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
				//
				//
				//
				//vk::AttachmentReference refResolve = { 2,vk::ImageLayout::eColorAttachmentOptimal };
			


				builder.ResolveOutput("Default",m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eClear);
				builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetInitialLayout(vk::ImageLayout::eUndefined);
				//builder.SetFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetFinalLayout(vk::ImageLayout::ePresentSrcKHR);
				auto resolve = builder.BuildAttachmentDesc();
				
				
			



				vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);
				vk::SubpassDependency dependency1 = SubpassDependency(0, 1,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);
				vk::SubpassDependency dependency2 = SubpassDependency(1,2,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);
				vk::SubpassDependency dependency3 = SubpassDependency(2, 3,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);


				builder.AddSubpass({ 0 }, { 1 }, { 2 });
				builder.AddSubpass({ 0 }, { 1 }, { 2 });
				builder.AddSubpass({ 0 }, { 1 }, { 2 });
				builder.AddSubpass({ 0 }, { 1 }, { 2 });

				builder.AddSubpassDependency(dependency0);
				builder.AddSubpassDependency(dependency1);
				builder.AddSubpassDependency(dependency2);
				builder.AddSubpassDependency(dependency3);


				auto exe = [this](CommandBuffer& commandBuffer, size_t frameIndex)
				{
					auto vkCommandBuffer = commandBuffer.GetCommandBuffer();
					std::string_view names[] = {BASIC_RENDER_PASS,CLIP_RENDER_PASS,NEW_PAGE_RENDER_PASS,NEW_PAGE_RIGHT_RENDER_PASS};
					auto size = std::size(names);
					Renderer::Instance()->BeginBatch(); 
					for (auto i = 0; i < size; i++)
					{
						auto pipeline = Renderer::Instance()->GetPipeline(names[i]);
						vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 0, m_DescriptorSets[frameIndex], nullptr); 
						vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 1, m_DescriptorSetTex, nullptr); 
						vkCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline()); 
						vk::Viewport viewport; 
						viewport.x = 0; 
						viewport.y = 0; 
						viewport.minDepth = 0; 
						viewport.maxDepth = 1; 
						viewport.width = Application::GetScreenWidth(); 
						viewport.height = Application::GetScreenHeight(); 
						vk::Rect2D scissors; 
						scissors.offset = vk::Offset2D{ (uint32_t)0,(uint32_t)0 }; 
						scissors.extent = vk::Extent2D{ (uint32_t)viewport.width,(uint32_t)viewport.height }; 
						vkCommandBuffer.setViewport(0, 1, &viewport);
						vkCommandBuffer.setScissor(0, 1, &scissors); 
						auto& drawables = Renderer::Instance()->GetDrawables(names[i]);
						for (auto& quad : drawables)
						{
							Renderer::Instance()->Draw(quad); 
						}
						auto offset = 0;
						//if (i == 1 || i == 2 || i == 3)
						//{
						//	offset = sizeof(Vertex) * 4;
						//}
						Renderer::Instance()->DrawBatchCustom(vkCommandBuffer,drawables.size()*6,0, drawables.size() * 6);
						if (i != size - 1)
						vkCommandBuffer.nextSubpass(vk::SubpassContents::eInline);

					}
				};

				m_RenderPass = builder.Build(BASIC_RENDER_PASS, m_AttachmentManager, actualFrameAmount, extent, clearValues, exe);

				
				

			}
			{


				RenderPassBuilder builder;
				builder.ColorOutput("MSAA", m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eLoad);
				builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetInitialLayout(vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

				auto msaaAttachment = builder.BuildAttachmentDesc();
				




				builder.DepthStencilOutput("DepthStencil", m_AttachmentManager, vk::ImageLayout::eDepthStencilAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eLoad);
				builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eLoad);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				builder.SetFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				auto depthAttachment = builder.BuildAttachmentDesc();
				

				builder.ResolveOutput("Default", m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eClear);
				builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetInitialLayout(vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
				

				vk::AttachmentDescription colorAttachmentResolve = builder.BuildAttachmentDesc();
			

				builder.AddSubpass({ 0 }, { 0 }, {0});

				vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);
				builder.AddSubpassDependency(dependency0);

				m_ClipRenderPass = builder.Build(CLIP_RENDER_PASS, m_AttachmentManager, actualFrameAmount, extent, clearValues, execute(CLIP_RENDER_PASS));
			}

			
			{
				RenderPassBuilder builder;
				builder.ColorOutput("MSAA", m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eLoad);
				builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetInitialLayout(vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

				auto msaaAttachment = builder.BuildAttachmentDesc();




				builder.DepthStencilOutput("DepthStencil", m_AttachmentManager, vk::ImageLayout::eDepthStencilAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eLoad);
				builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eLoad);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				builder.SetFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				auto depthAttachment = builder.BuildAttachmentDesc();




				builder.ResolveOutput("Default", m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eClear);
				builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetInitialLayout(vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
				vk::AttachmentDescription colorAttachmentResolve = builder.BuildAttachmentDesc();


				builder.AddSubpass({ 0 }, { 0 }, { 0 });

				vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);
				builder.AddSubpassDependency(dependency0);

				m_NewPageRenderPass = builder.Build(NEW_PAGE_RENDER_PASS,m_AttachmentManager, actualFrameAmount, extent, clearValues, execute(NEW_PAGE_RENDER_PASS));
				
			}

			{
				RenderPassBuilder builder;

				builder.ColorOutput("MSAA", m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eLoad);
				builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetInitialLayout(vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

				auto msaaAttachment = builder.BuildAttachmentDesc();





				builder.DepthStencilOutput("DepthStencil", m_AttachmentManager, vk::ImageLayout::eDepthStencilAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eLoad);
				builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eLoad);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				builder.SetFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				auto depthAttachment = builder.BuildAttachmentDesc();



				builder.ResolveOutput("Default", m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eClear);
				builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetInitialLayout(vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
				vk::AttachmentDescription colorAttachmentResolve = builder.BuildAttachmentDesc();

				//Renderpasses are broken down into subpasses, there's always at least one.

				builder.AddSubpass({ 0 }, { 0 }, { 0 });

				vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);
				builder.AddSubpassDependency(dependency0);
				m_NewPageRenderPassRight = builder.Build(NEW_PAGE_RIGHT_RENDER_PASS,m_AttachmentManager, actualFrameAmount, extent, clearValues, execute(NEW_PAGE_RIGHT_RENDER_PASS));
			}
			
			
			std::vector<vk::VertexInputBindingDescription> bindings{ VertexBindingDescription(0,sizeof(Vertex),vk::VertexInputRate::eVertex) };

			std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;

			attributeDescriptions =
			{
				VertexInputAttributeDescription(0,0,vk::Format::eR32G32B32Sfloat,offsetof(Vertex, Position)),
				VertexInputAttributeDescription(0,1,vk::Format::eR32G32Sfloat,offsetof(Vertex, UV)),
				VertexInputAttributeDescription(0,2,vk::Format::eR32G32B32A32Sfloat,offsetof(Vertex, Color)),
				VertexInputAttributeDescription(0,3,vk::Format::eR32Sfloat,offsetof(Vertex, textureID)),

			};
			


			{
				
				auto pipelineLayouts = std::vector<vk::DescriptorSetLayout>{ m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutTex->GetLayout() };

				PipelineBuilder builder;
				builder.SetDevice(device->GetDevice());
				builder.SetSamples(samples);
				builder.AddDescriptorLayouts(pipelineLayouts);
				builder.AddAttributeDescription(attributeDescriptions);
				builder.AddBindingDescription(bindings);
				builder.SetPolygoneMode(Renderer::Instance()->GetPolygonMode());
				builder.SetTopology(vk::PrimitiveTopology::eTriangleList);
				Renderer::Instance()->CompileShader("default.spvV", ShaderType::VERTEX);
				Renderer::Instance()->CompileShader("page.spvF", ShaderType::FRAGMENT);
				builder.AddShader(BASE_SPIRV_OUTPUT + "default.spvV", vk::ShaderStageFlagBits::eVertex);
				builder.AddShader(BASE_SPIRV_OUTPUT + "page.spvF", vk::ShaderStageFlagBits::eFragment);
				builder.SetSubpassAmount(0);
				builder.AddExtent(extent);
				builder.AddImageFormat(vk::Format::eB8G8R8A8Unorm);
				builder.EnableStencilTest(true);
				builder.SetDepthTest(true);
				builder.WriteToDepthBuffer(true);
				builder.SetRenderPass(m_RenderPass->GetRaw());
				builder.SetStencilRefNumber(2);
				builder.StencilTestOp(vk::CompareOp::eAlways,vk::StencilOp::eReplace, vk::StencilOp::eReplace, vk::StencilOp::eReplace);
				builder.SetMasks(0xff, 0xff);

				builder.Build(BASIC_RENDER_PASS);


				{
					PipelineBuilder builder;
					builder.SetDevice(device->GetDevice());
					builder.SetSamples(samples);
					builder.AddDescriptorLayouts(pipelineLayouts);
					builder.AddAttributeDescription(attributeDescriptions);
					builder.AddBindingDescription(bindings);
					builder.SetPolygoneMode(Renderer::Instance()->GetPolygonMode());
					builder.SetTopology(vk::PrimitiveTopology::eTriangleList);
					builder.AddShader(BASE_SPIRV_OUTPUT + "default.spvV", vk::ShaderStageFlagBits::eVertex);
					builder.AddShader(BASE_SPIRV_OUTPUT + "page.spvF", vk::ShaderStageFlagBits::eFragment);
					builder.SetSubpassAmount(1);
					builder.AddExtent(extent);
					builder.AddImageFormat(vk::Format::eB8G8R8A8Unorm);
					builder.EnableStencilTest(true);
					builder.SetDepthTest(false);
					builder.WriteToDepthBuffer(true);
					builder.SetRenderPass(m_RenderPass->GetRaw());
					builder.SetStencilRefNumber(2);
					builder.StencilTestOp(vk::CompareOp::eEqual, vk::StencilOp::eKeep, vk::StencilOp::eDecrementAndClamp, vk::StencilOp::eKeep);
					builder.SetMasks(0xff, 0xff);
					builder.Build(CLIP_RENDER_PASS);
				}

				{
					{
						PipelineBuilder builder;
						builder.SetDevice(device->GetDevice());
						builder.SetSamples(samples);
						builder.AddDescriptorLayouts(pipelineLayouts);
						builder.AddAttributeDescription(attributeDescriptions);
						builder.AddBindingDescription(bindings);
						builder.SetPolygoneMode(Renderer::Instance()->GetPolygonMode());
						builder.SetTopology(vk::PrimitiveTopology::eTriangleList);
						Renderer::Instance()->CompileShader("leftNewPage.spvF", ShaderType::FRAGMENT);
						builder.AddShader(BASE_SPIRV_OUTPUT + "default.spvV", vk::ShaderStageFlagBits::eVertex);
						builder.AddShader(BASE_SPIRV_OUTPUT + "leftNewPage.spvF", vk::ShaderStageFlagBits::eFragment);
						builder.SetSubpassAmount(2);
						builder.AddExtent(extent);
						builder.AddImageFormat(vk::Format::eB8G8R8A8Unorm);
						builder.EnableStencilTest(true);
						builder.SetDepthTest(false);
						builder.WriteToDepthBuffer(true);
						builder.SetRenderPass(m_RenderPass->GetRaw());
						builder.SetStencilRefNumber(1);
						builder.StencilTestOp(vk::CompareOp::eEqual, vk::StencilOp::eKeep, vk::StencilOp::eReplace, vk::StencilOp::eKeep);
						builder.SetMasks(0xff, 0xff);
						builder.Build(NEW_PAGE_RENDER_PASS);

						builder.DestroyShaderModules();
						builder.AddShader(BASE_SPIRV_OUTPUT + "default.spvV", vk::ShaderStageFlagBits::eVertex);
						Renderer::Instance()->CompileShader("rightNewPage.spvF", ShaderType::FRAGMENT);
						builder.SetSubpassAmount(3);
						builder.SetRenderPass(m_RenderPass->GetRaw());
						builder.AddShader(BASE_SPIRV_OUTPUT + "rightNewPage.spvF", vk::ShaderStageFlagBits::eFragment);
						builder.SetStencilRefNumber(2);
						builder.StencilTestOp(vk::CompareOp::eEqual, vk::StencilOp::eKeep, vk::StencilOp::eReplace, vk::StencilOp::eKeep);
						builder.SetBlendOp(vk::BlendOp::eAdd, vk::BlendFactor::eOne,vk::BlendFactor::eZero);
						builder.Build(NEW_PAGE_RIGHT_RENDER_PASS);
					}
				}
				
				

			}
			

		

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

			UPtr<RenderPass> m_ImGuiRenderPass = builder.Build(IMGUI_RENDER_PASS, m_AttachmentManager,
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
			result = ImGui_ImplVulkan_Init(&init_info, m_ImGuiRenderPass->GetRaw());
			auto commandBuffer = Renderer::Instance()->GetTransferCommandBuffer(0);
			commandBuffer.BeginTransfering();
			result = ImGui_ImplVulkan_CreateFontsTexture((VkCommandBuffer)commandBuffer.GetCommandBuffer());
			commandBuffer.EndTransfering();
			commandBuffer.SubmitSingle();
			ImGui_ImplVulkan_DestroyFontUploadObjects();


			auto graph = CreateUPtr<RenderPassGraph>();
			graph->AddRenderPass(std::move(m_RenderPass));
			//graph->AddRenderPass(std::move(m_ClipRenderPass));
			//graph->AddRenderPass(std::move(m_NewPageRenderPass));
			//graph->AddRenderPass(std::move(m_NewPageRenderPassRight));
			graph->AddRenderPass(std::move(m_ImGuiRenderPass));
			graph->AddExec(BASIC_RENDER_PASS);
			//graph->AddExec(CLIP_RENDER_PASS);
			//graph->AddExec(NEW_PAGE_RENDER_PASS);
			//graph->AddExec(NEW_PAGE_RIGHT_RENDER_PASS);
			//graph->AddExec(IMGUI_RENDER_PASS);

			Renderer::Instance()->AddRenderGraph("",std::move(graph));



			m_AttachmentManager.CreateColor(PAGE_RENDER_PASS, m_AttachmentManager, vk::Format::eB8G8R8A8Unorm,
				PageRenderWidth, PageRenderHeight, vk::SampleCountFlagBits::e1,
				vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst |
				vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled, PageAmount);
			UPtr<RenderPass> m_PageRenderPass;
			{
				RenderPassBuilder builder;

				builder.ColorOutput(PAGE_RENDER_PASS, m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetLoadOp(vk::AttachmentLoadOp::eClear);
				builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
				builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
				builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
				builder.SetInitialLayout(vk::ImageLayout::eUndefined);
				builder.SetFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
				vk::AttachmentDescription colorAttachmentResolve = builder.BuildAttachmentDesc();

				builder.AddSubpass({ 0 }, { 0 }, { 0 });
				vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eNone, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);
				builder.AddSubpassDependency(dependency0);
				vk::ClearValue clearColor = { std::array<float, 4>{0.f, 0.f, 0.f, 1.0f} };


				std::vector<vk::ClearValue> clearValues = { {clearColor,clearColor} };

				m_PageRenderPass = builder.Build(PAGE_RENDER_PASS, m_AttachmentManager, PageAmount, { (uint32_t)PageRenderWidth, (uint32_t)PageRenderHeight }, clearValues,
					[this](CommandBuffer& cmd, size_t frameIndex)
					{
						auto vkCommandBuffer = cmd.GetCommandBuffer();
						glm::vec2 pos = { 20, 400 };

						auto pipeline = Renderer::Instance()->GetPipeline(PAGE_RENDER_PASS);
						vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 0, m_DescriptorSets[0], nullptr);
						vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 1, m_DescriptorSetFont, nullptr);


						vkCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());

						auto viewportSize = std::make_pair(PageRenderWidth, PageRenderHeight);
						vk::Viewport viewport;
						viewport.x = 0;
						viewport.y = 0;
						viewport.minDepth = 0;
						viewport.maxDepth = 1;
						viewport.height = viewportSize.second;
						viewport.width = viewportSize.first;

						vk::Rect2D scissors;
						scissors.offset = vk::Offset2D{ (uint32_t)0,(uint32_t)0 };
						scissors.extent = vk::Extent2D{ (uint32_t)viewportSize.first,(uint32_t)viewportSize.second };

						vkCommandBuffer.setViewport(0, 1, &viewport);
						vkCommandBuffer.setScissor(0, 1, &scissors);
						Renderer::Instance()->BeginBatch();
						Renderer::Instance()->DrawTxt(vkCommandBuffer, txt[frameIndex % txt.size()], pos, Characters);
						size_t offset = 0;
						Renderer::Instance()->DrawBatch(vkCommandBuffer, offset);
					}

				);
			}

			{
				auto m_DescriptorSetLayout = Renderer::Instance()->GetSetLayout(0, PipelineType::RENDER);
				auto m_DescriptorSetLayoutFont = Renderer::Instance()->GetSetLayout(m_TexAtlas, PipelineType::RENDER);

				auto samples = RenderContext::GetDevice()->GetSamples();
				auto pipelineLayouts = std::vector<vk::DescriptorSetLayout>{
					m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutFont->GetLayout() };
				std::vector<vk::VertexInputBindingDescription> bindings{ VertexBindingDescription(0,sizeof(Vertex),vk::VertexInputRate::eVertex) };

				std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;

				attributeDescriptions =
				{
					VertexInputAttributeDescription(0,0,vk::Format::eR32G32B32Sfloat,offsetof(Vertex, Position)),
					VertexInputAttributeDescription(0,1,vk::Format::eR32G32Sfloat,offsetof(Vertex, UV))
				};

				PipelineBuilder builder;
				builder.SetDevice(device->GetDevice());
				builder.SetSamples(vk::SampleCountFlagBits::e1);
				builder.AddDescriptorLayouts(pipelineLayouts);
				builder.AddAttributeDescription(attributeDescriptions);
				builder.AddBindingDescription(bindings);
				builder.SetPolygoneMode(Renderer::Instance()->GetPolygonMode());
				builder.SetTopology(vk::PrimitiveTopology::eTriangleList);
				Renderer::Instance()->CompileShader("font.spvV", ShaderType::VERTEX);
				Renderer::Instance()->CompileShader("font.spvF", ShaderType::FRAGMENT);
				builder.AddShader(BASE_SPIRV_OUTPUT + "font.spvV", vk::ShaderStageFlagBits::eVertex);
				builder.AddShader(BASE_SPIRV_OUTPUT + "font.spvF", vk::ShaderStageFlagBits::eFragment);
				builder.SetSubpassAmount(0);

				builder.AddExtent(vk::Extent2D{ 128 ,128 });
				builder.AddImageFormat(vk::Format::eB8G8R8A8Unorm);
				builder.SetRenderPass(m_PageRenderPass->GetRaw());

				//builder.SetSampleShading(VK_TRUE);

				builder.Build(PAGE_RENDER_PASS);
				auto pagePipeline = Renderer::Instance()->GetPipeline(PAGE_RENDER_PASS);

			}



			auto camera = *GetCamera();
			auto& cmd = Renderer::Instance()->GetRenderCommandBuffer(0);

			Fence fence;
			for (int i = 0; i < PageAmount; i++)
			{
				Renderer::Instance()->BeginFrame(camera, PageRenderWidth, PageRenderHeight);
				Renderer::Instance()->Wait(fence.GetFence());
				Renderer::Instance()->Reset(fence.GetFence());
				m_PageRenderPass->Execute(cmd, i);

				cmd.Submit(&fence.GetFence());
			}

			{
				std::vector<vk::DescriptorImageInfo> infos(PageAmount);
				auto attach = m_AttachmentManager.GetColor({ PAGE_RENDER_PASS });
				for (int i = 0; i < PageAmount; i++)
				{
					infos[i].sampler = attach[i]->GetSampler();
					infos[i].imageView = attach[i]->GetImageView();
					infos[i].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

				}
				device->UpdateDescriptorSet(m_DescriptorSetTex, 0, infos, vk::DescriptorType::eCombinedImageSampler);

			}

					
		};

		auto cleanup = [this]()
		{



				auto device = RenderContext::GetDevice()->GetDevice();
				device.waitIdle();
				m_ShaderStorageBuffer.reset();
				m_MouseInfo.clear();
				m_Image.reset();
				m_ImageSelected.reset();
				m_FontAtlas.reset();
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
	void UpdateBuffer(Device& device, vk::DescriptorSet set, void* cpuBuffer, Buffer& gpuBuffer, int size )
	{
			SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(size);


			auto transferCommandBuffer = Renderer::Instance()->GetTransferCommandBuffer(0);

			transferCommandBuffer.BeginTransfering();
			transferCommandBuffer.Transfer(stagingBuffer.get(), m_ShaderStorageBuffer.get(), (void*)m_ClickPoints.data(), size);
			transferCommandBuffer.EndTransfering();
			transferCommandBuffer.SubmitSingle();


			device.UpdateDescriptorSet(m_DescriptorSetSelected, 1, 1, *m_ShaderStorageBuffer, vk::DescriptorType::eStorageBuffer);
	}

	
	void UpdateTexture(size_t frameIndex)
	{
		/*auto device = RenderContext::GetDevice()->GetDevice();
		auto currentFrame = frameIndex;
		auto computeCommandBuffer = Renderer::Instance()->GetComputeCommandBuffer(frameIndex);
		if (m_ImageSelected->GetLayout() != vk::ImageLayout::eGeneral)
		{
			auto cmdBuffer = computeCommandBuffer.BeginTransfering();


			computeCommandBuffer.ChangeImageLayout(m_ImageSelected.get(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral);
			computeCommandBuffer.EndTransfering();
			computeCommandBuffer.SubmitSingle();

		}
		auto cmdBuffer = computeCommandBuffer.BeginTransfering();




		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline->GetPipeline());
		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_ComputePipeline->GetLayout(), 0, 1, &m_DescriptorSets[currentFrame], 0, 0);
		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_ComputePipeline->GetLayout(), 1, 1, &m_DescriptorSetSelected, 0, 0);
		float invocations = 256;
		int localSize = 8;


		vkCmdDispatch(cmdBuffer, invocations / localSize, invocations / localSize, 1);

		computeCommandBuffer.ChangeImageLayout(m_ImageSelected.get(), vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal);
		computeCommandBuffer.EndTransfering();
		computeCommandBuffer.SubmitSingle();


		device.waitIdle();*/
	}


	void LoadFont(std::string_view str)
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
		m_FontAtlas = Image::CreateEmptyImage(maxWidthTexture, maxHeight, vk::Format::eR8Unorm, usage);

		auto computeCommandBuffer = Renderer::Instance()->GetComputeCommandBuffer(0);

		computeCommandBuffer.BeginTransfering();
		computeCommandBuffer.ChangeImageLayout(m_FontAtlas.get(), vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferDstOptimal);
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
			computeCommandBuffer.CopyBufferToImage(*buffer.get(), m_FontAtlas->GetImage(), face->glyph->bitmap.width, face->glyph->bitmap.rows, { increment_x,increment_y,0 });
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
		computeCommandBuffer.ChangeImageLayout(m_FontAtlas.get(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
		computeCommandBuffer.EndTransfering();
		computeCommandBuffer.SubmitSingle();


	}

	void PreRender(Camera& camera) override
	{
		const auto scale = glm::vec3(glm::vec3(400, 512, 0));

#define LEFT_PAGE_COLOR whiteColor
#define RIGHT_PAGE_COLOR redColor
#define RIGHT_NEW_PAGE_COLOR blueColor
#define LEFT_NEW_PAGE_COLOR greyColor
		const float width = scale.x;
		const float height = scale.y;
		const float startPointX = 150 * 2.f;
		const float startPointY = 200;

		const glm::vec2 leftEdgeBottom = { startPointX  ,startPointY };
		const glm::vec2 spineTop = { startPointX + width ,startPointY + height };
		const glm::vec2 spineBottom = { startPointX + width,startPointY };
		const glm::vec2 center = { startPointX + width / 2,startPointY + height / 2 };
		const glm::vec2 rightEdgeBottom = { width + startPointX + width,startPointY };
		const glm::vec2 rightEdgeTop = { width + startPointX + width,startPointY + height };

		
		{
			ZoneScopedN("Sumbit render commands");


			auto vecPos = glm::vec4(0.5, -0.5, 0, 1) * 10.f + glm::vec4(100, 100, 0, 0);
			vecPos.w = 1;
			auto proj = camera.GetProj();
			auto res = proj * vecPos;



			auto& world = glm::translate(glm::identity<glm::mat4>(), glm::vec3(500, 300, 0));
			world = glm::scale(world, scale);
			auto world2 = glm::translate(glm::identity<glm::mat4>(), glm::vec3(world[3]) + glm::vec3(scale.x, 0, 0));
			world2 = glm::scale(world2, scale);

			auto  GetMousePosition = [](Camera& camera)
				{
					auto  mousePos = Input::GetMousePos();
					glm::vec2 screenSize = { Renderer::Instance()->GetViewportSize().first,
						Renderer::Instance()->GetViewportSize().second };
					glm::vec3 ndc;
					auto mouseScreenPos = glm::vec2{ std::get<0>(mousePos),std::get<1>(mousePos) };
					ndc.x = (2.0f * mouseScreenPos.x / screenSize.x) - 1.0f;
					ndc.y = -(1.0f - (2.0f * mouseScreenPos.y / screenSize.y));
					auto inverseProj = glm::inverse(camera.GetProj());
					auto clipSpace = (inverseProj * glm::vec4{ ndc.x,ndc.y,1,1 });

					auto follow = glm::vec2(clipSpace);
					return follow;
				};

			if (Input::IsMousePressed(0))
			{
				if (!isClicked)
				{
					isClicked = true;
					isDragged = true;
				}
				m_Follow = GetMousePosition(camera);
			}
			else
			{
				if (isDragged && glm::length(GetMousePosition(camera) - leftEdgeBottom) < 15 * 2)
				{
					std::swap(LEFT_PAGE_COLOR, LEFT_NEW_PAGE_COLOR);
					std::swap(textureIndicies[0], textureIndicies[2]);
					std::swap(RIGHT_PAGE_COLOR, RIGHT_NEW_PAGE_COLOR);
					std::swap(textureIndicies[1], textureIndicies[3]);
				}
				isClicked = false;
				isDragged = false;
				m_Follow = rightEdgeBottom;
			}
			m_Follow = glm::clamp(m_Follow, leftEdgeBottom, glm::vec2{ rightEdgeBottom.x - 0,center.y - 40 });

			glm::vec2 t0;
			t0.x = m_Follow.x + .5 * (rightEdgeBottom.x - m_Follow.x);
			t0.y = m_Follow.y + .5 * (rightEdgeBottom.y - m_Follow.y);
			t0 = glm::clamp(t0, spineBottom, rightEdgeTop);
			auto bisectorAngle = glm::atan2(rightEdgeBottom.y - t0.y, rightEdgeBottom.x - t0.x);
			//t1
			auto bisectorTangent = t0.x - glm::tan(bisectorAngle) * (rightEdgeBottom.y - t0.y);
			//bisectorTangent = glm::max(bisectorTangent, spineBottom.x);
			if (bisectorTangent < spineBottom.x) bisectorTangent = spineBottom.x - 2;
			if (bisectorTangent > rightEdgeBottom.x) bisectorTangent = rightEdgeBottom.x - 2;
			glm::vec2 t1;
			glm::vec2 t2;
			t1.x = bisectorTangent;
			t1.y = rightEdgeBottom.y;
			t2.x = t0.x;
			t2.y = t1.y;
			t1 = glm::clamp(t1, spineBottom, rightEdgeBottom);
			t2 = glm::clamp(t2, spineBottom, rightEdgeBottom);
			const auto PI2 = 3.14f / 2.f;
			const auto PI = 3.14f;
			auto deltaXPage = t1.x - m_Follow.x;
			auto deltaYPage = -t1.y + m_Follow.y;
			float pageAngle = glm::atan2(deltaXPage, deltaYPage);
			pageAngle = (PI2 + pageAngle);
			if (t1.x > rightEdgeBottom.x)
			{
				//std::cout << "as";
			}
			if (deltaXPage == 0)
			{
				pageAngle = 0;
			}
			pageAngle = glm::clamp(pageAngle, 0.f, PI);

			//std::cout << "Follow" << m_Follow.x << " " << m_Follow.y << std::endl;
			//std::cout << "t1 " << t1.x << " " << t1.y << std::endl;
			//pageAngle = -(3.14);
			//std::cout << "dx " << deltaXPage << std::endl;
			//std::cout << "dy " << deltaYPage << std::endl;
			//std::cout << "Page angle " << glm::degrees(pageAngle) << std::endl;
			//pageAngle = glm::pi<float>() - pageAngle;
			auto deltaXClip = (t1.x - t0.x);
			auto deltaYClip = -t1.y + t0.y;
			auto clipAngle = glm::atan2(deltaXClip, deltaYClip);
			if (deltaXClip == 0)

			{
				//clipAngle = -PI2;
			}
			/*if (clipAngle< -PI2)
			{
				clipAngle = -PI2;
			}*/
			clipAngle += PI2;
			//clipAngle *= -1;
			//std::cout << "dx " << deltaXClip << std::endl;
			//std::cout << "dy " << deltaYClip  << std::endl;
			//std::cout << "Clip angle " << glm::degrees(clipAngle) << std::endl;
			if (pageAngle > (3.14 / 2))
			{
				//pageAngle -= 3.14 ;
			}
			//std::cout << "mouse " << follow.x << " " << follow.y << "\n";
			//std::cout << "t0 " << t0.x << " " << t0.y << "\n";
			//std::cout << "t1 " << t1.x << " " << t1.y << "\n";
			std::vector<Vertex> leftPage{ 4 };
			leftPage[0].Position = { startPointX,startPointY,0 };
			leftPage[0].Color = LEFT_PAGE_COLOR;
			leftPage[1].Position = { startPointX,startPointY + height,0 };
			leftPage[1].Color = LEFT_PAGE_COLOR;
			leftPage[2].Position = { startPointX + width,startPointY,0 };
			leftPage[2].Color = LEFT_PAGE_COLOR;
			leftPage[3].Position = { startPointX + width,startPointY + height,0 };
			leftPage[3].Color = LEFT_PAGE_COLOR;
			leftPage[0].textureID = leftPage[1].textureID = leftPage[2].textureID = leftPage[3].textureID = textureIndicies[0];
			std::vector<Vertex> rightPage{ 4 };
			rightPage[0].Position = { width + startPointX,startPointY,0 };
			rightPage[0].Color = RIGHT_PAGE_COLOR;
			rightPage[1].Position = { width + startPointX,startPointY + height,0 };
			rightPage[1].Color = RIGHT_PAGE_COLOR;
			rightPage[2].Position = { 2 * width + startPointX ,startPointY,0 };
			rightPage[2].Color = RIGHT_PAGE_COLOR;
			rightPage[3].Position = { glm::vec2{2 * width + startPointX,startPointY + height},0 };
			rightPage[3].Color = RIGHT_PAGE_COLOR;
			rightPage[0].textureID = rightPage[1].textureID = rightPage[2].textureID = rightPage[3].textureID = textureIndicies[1];

			std::vector<Vertex> newLeftPage{ 4 };
			//newLeftPage[0].Position = { 2 * width + startPointX,startPointY,0 };
			newLeftPage[0].Position = { m_Follow,0 };
			newLeftPage[0].Color = LEFT_NEW_PAGE_COLOR;
			newLeftPage[1].Position = { newLeftPage[0].Position.x,newLeftPage[0].Position.y - height,0 };
			newLeftPage[1].Color = LEFT_NEW_PAGE_COLOR;
			newLeftPage[2].Position = { newLeftPage[0].Position.x - width ,newLeftPage[0].Position.y,0 };
			newLeftPage[2].Color = LEFT_NEW_PAGE_COLOR;
			newLeftPage[3].Position = { newLeftPage[0].Position.x - width,newLeftPage[0].Position.y - height ,0 };
			newLeftPage[3].Color = LEFT_NEW_PAGE_COLOR;
			newLeftPage[0].textureID = newLeftPage[1].textureID = newLeftPage[2].textureID = newLeftPage[3].textureID = textureIndicies[2];

			std::vector<Vertex> newRightPage{ 4 };
			newRightPage[0].Position = { width + startPointX,startPointY,-0.5 };
			newRightPage[0].Color = RIGHT_NEW_PAGE_COLOR;
			newRightPage[1].Position = { width + startPointX,startPointY + height,-0.5 };
			newRightPage[1].Color = RIGHT_NEW_PAGE_COLOR;
			newRightPage[2].Position = { 2 * width + startPointX ,startPointY,-0.5 };
			newRightPage[2].Color = RIGHT_NEW_PAGE_COLOR;
			newRightPage[3].Position = { glm::vec2{2 * width + startPointX,startPointY + height},-0.5 };
			newRightPage[3].Color = RIGHT_NEW_PAGE_COLOR;
			newRightPage[0].textureID = newRightPage[1].textureID = newRightPage[2].textureID = newRightPage[3].textureID = textureIndicies[3];









			auto rotateVector = [](glm::vec3 v, float radians, glm::vec2 pointOfRotation) {
				double dx = v.x - pointOfRotation.x;
				double dy = v.y - pointOfRotation.y;

				double new_x = dx * cos(radians) - dy * sin(radians) + pointOfRotation.x;
				double new_y = dx * sin(radians) + dy * cos(radians) + pointOfRotation.y;
				v = glm::vec3{ new_x,new_y ,0 };
				return v;
				};
			auto getCenter = [](std::vector<Vertex>& verticies, double& centerX, double& centerY)
				{
					for (int i = 0; i < 4; i++) {
						centerX += verticies[i].Position.x;
						centerY += verticies[i].Position.y;
					}

					centerX /= 4.0;
					centerY /= 4.0;
				};
			double centerX = 0.0;
			double centerY = 0.0;
			getCenter(newLeftPage, centerX, centerY);




			glm::vec2 pointOfRotation = newLeftPage[0].Position;
			//pointOfRotation = { centerX,centerY };

			// pointOfRotation = {0,0};
			 //pointOfRotation = spineBottom;
			//pageAngle += 2* 3.14 / 2;
			for (int i = 0; i < 4; i++) {
				newLeftPage[i].Position = rotateVector(newLeftPage[i].Position, pageAngle, pointOfRotation);
			}

			std::vector<Vertex> clipPage{ 4 };

			float clipHeight = 600;
			float clipWidth = 600;
			auto modifiedBlueColor = blueColor;
			modifiedBlueColor.a = 0;
			clipPage[0].Position = { t1.x - clipWidth  ,t1.y + clipHeight ,0 };
			clipPage[0].Color = modifiedBlueColor;
			clipPage[1].Position = { t1.x + clipWidth,t1.y + clipHeight ,0 };
			clipPage[1].Color = modifiedBlueColor;
			clipPage[2].Position = { t1.x - clipWidth   ,t1.y ,0 };
			clipPage[2].Color = modifiedBlueColor;
			clipPage[3].Position = { t1.x + clipWidth,t1.y ,0 };
			clipPage[3].Color = modifiedBlueColor;

			for (int i = 0; i < 4; i++) {
				clipPage[i].Position = rotateVector(clipPage[i].Position, clipAngle, t1);
			}


		
	
			std::vector<std::vector<Vertex>> pages =
			{
				leftPage,
				rightPage,
				newLeftPage,
				newRightPage
			};

			Renderer::Instance()->AddDrawable(BASIC_RENDER_PASS, QuadRangle{ (pages[0]) });
			Renderer::Instance()->AddDrawable(BASIC_RENDER_PASS, QuadRangle{ (pages[1]) });
			Renderer::Instance()->AddDrawable(CLIP_RENDER_PASS, QuadRangle{ (clipPage) });
			Renderer::Instance()->AddDrawable(NEW_PAGE_RENDER_PASS, QuadRangle{ (pages[2]) });
			Renderer::Instance()->AddDrawable(NEW_PAGE_RIGHT_RENDER_PASS, QuadRangle{ (pages[3]) });
			static size_t currentFrame = 0;
			AdditionalData data;
			data.mouseFollow = m_Follow;
			auto ptr = RenderContext::GetDevice()->GetDevice().mapMemory(m_MouseInfo[currentFrame]->GetMemory(), 0, sizeof AdditionalData);
			memcpy(ptr,&data,sizeof AdditionalData);
			RenderContext::GetDevice()->GetDevice().unmapMemory(m_MouseInfo[currentFrame]->GetMemory());

			RenderContext::GetDevice()->UpdateDescriptorSet(m_DescriptorSets[currentFrame],1,1,
				*m_MouseInfo[currentFrame], vk::DescriptorType::eUniformBuffer);
			currentFrame++;
			currentFrame %= RenderContext::GetFrameAmount();
		}
	}



private:
	struct AdditionalData
	{
		glm::vec2 mouseFollow;
	};
	int m_BaseDesc = 0;
	int m_TexDesc = 0;
	int m_Compute = 0;
	int m_TexAtlas = 0;
	const uint32_t MAX_POINTS = 20;
	int nextPoint = 0;
	std::vector<vk::DescriptorSet> m_DescriptorSets;
	vk::DescriptorSet m_DescriptorSetTex;
	vk::DescriptorSet m_DescriptorSetSelected;
	vk::DescriptorSet m_DescriptorSetFont;
	glm::vec2 m_Follow;

	
	std::vector<UPtr<Buffer>> m_MouseInfo;
	UPtr<Buffer> m_ShaderStorageBuffer;
	SPtr<Image> m_Image;
	SPtr<Image> m_ImageSelected;
	SPtr<Image> m_FontAtlas;
	
	std::vector<glm::vec2> m_ClickPoints;


	bool isClicked = false;
	bool isDragged = false;
	glm::vec4 whiteColor = { 0.2,0.7,0.2,1 }; // left
	glm::vec4 redColor = { 0.6,0,0,1 }; // right
	glm::vec4 greyColor = { 0.7,0.4,0.7,1 }; // new left
	glm::vec4 blueColor = { 0.2,0.2,0.7,1 }; // new right

	std::string PageStr1= "Hello, traveller!\nHow is it going?\nHave you found\nwhat you seek?\n\nAnyway...";
	std::string PageStr2 = "Here is page\ncurl rendering\n with Vulkan!\nThe text is rendered\nwith Vulkan too!";
	std::string PageStr3 = "This page rendering\nwas inspired by\nsimiliar effect in\n beloved\nDivinity: Original\n Sin Games";
	std::string PageStr4 = "May the force\nbe with you!\nHopefully this book\nenriched your\n potential!";
	std::vector<std::string> txt = { PageStr1,PageStr2,PageStr3,PageStr4 };
	std::vector<float> textureIndicies = { 0,1,2,3};
	Callables callables;
	AttachmentManager m_AttachmentManager;


};


Voidstar::Application* Voidstar::CreateApplication()
{
	auto str = std::string("Example");
	// 120 -> 1920 * 1080
	// 110
	const int res = 90;
	return new ExampleApplication(str, 16 * res, 9 * res);
}
int main()
{
	return Main();
}
