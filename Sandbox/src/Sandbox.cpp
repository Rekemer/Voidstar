

#include <iostream>
#include <vector>

#include"Voidstar.h"
#include"tracy/Tracy.hpp"
#include"tracy/TracyVulkan.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H  
#include <map>
#include <utility>
#include "gtx/compatibility.hpp"
using namespace Voidstar;


std::map<unsigned char, Character> Characters;
const float PageRenderWidth = 400.f;
const float PageRenderHeight = 512.f;
const int PageAmount = 4;


class ExampleApplication : public Voidstar::Application
{
public:
	ExampleApplication(std::string appName, size_t screenWidth, size_t screenHeight) : Voidstar::Application(appName, screenWidth, screenHeight)
	{
		// init renderer

		
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

		auto bufferInit  = [this]()
		{
			// get frame amount
			auto framesAmount = Renderer::Instance()->GetSwapchain().GetFrameAmount();
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



			



			{
				BufferInputChunk info;
				info.size = sizeof(glm::vec2) * MAX_POINTS;
				info.usage = vk::BufferUsageFlagBits::eStorageBuffer |
					vk::BufferUsageFlagBits::eTransferDst;
				info.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible |
					vk::MemoryPropertyFlagBits::eHostCoherent;
				m_ShaderStorageBuffer = CreateUPtr<Buffer>(info);
			}
		};



		

		auto loadTextures = [this]()
		{
			const float noiseTextureWidth = 256.f;
			const float noiseTextureHeight = 256.f;
			auto usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
			m_ImageSelected = Image::CreateEmptyImage(noiseTextureWidth, noiseTextureHeight, vk::Format::eR8G8B8A8Snorm,usage);
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
			auto framesAmount = Renderer::Instance()->GetSwapchain().GetFrameAmount();
			for (size_t i = 0; i < framesAmount; i++)
			{
				device->UpdateDescriptorSet(m_DescriptorSets[i], 0, 1, *m_UniformBuffers[i], vk::DescriptorType::eUniformBuffer);
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
			auto m_DescriptorSetLayoutTex = Renderer::Instance()->GetSetLayout(1, PipelineType::RENDER);
			auto m_DescriptorSetLayoutSelected = Renderer::Instance()->GetSetLayout(0, PipelineType::COMPUTE);
			auto m_DescriptorSetLayoutFont = Renderer::Instance()->GetSetLayout(m_TexAtlas, PipelineType::RENDER);
			
			
			
			std::vector< vk::DescriptorSetLayout>layouts = { m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutSelected->GetLayout() };

			Renderer::Instance()->CompileShader("SelectedTex.spvCmp",ShaderType::COMPUTE);
			m_ComputePipeline = Pipeline::CreateComputePipeline(BASE_SPIRV_OUTPUT + "SelectedTex.spvCmp", layouts);

			auto& swapchain = Renderer::Instance()->GetSwapchain();
			auto swapchainFormat = swapchain.GetFormat();
			auto swapChainExtent = swapchain.GetExtent();
			auto swapChainDepthFormat = swapchain.GetDepthFormat();


			auto device = RenderContext::GetDevice();

			// render pass
			auto samples = RenderContext::GetDevice()->GetSamples();
			
			{
				RenderPassBuilder builder;
				//Define a general attachment, with its load/store operations
				vk::AttachmentDescription msaaAttachment = AttachmentDescription(swapchainFormat,
					samples, vk::AttachmentLoadOp::eClear, 
					vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, 
					vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, 
					vk::ImageLayout::eColorAttachmentOptimal);

				vk::AttachmentReference refMSAA = { 0,vk::ImageLayout::eColorAttachmentOptimal };
				builder.AddAttachment({ msaaAttachment ,refMSAA });

				vk::AttachmentDescription depthAttachment = AttachmentDescription(
					swapChainDepthFormat, samples, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore
					, vk::AttachmentLoadOp::eClear,
					vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined,
					vk::ImageLayout::eDepthStencilAttachmentOptimal);
				vk::AttachmentReference refDepth = { 1,vk::ImageLayout::eDepthStencilAttachmentOptimal };
				builder.AddAttachment({ depthAttachment , refDepth });


#if IMGUI_ENABLED

				//Define a general attachment, with its load/store operations
				vk::AttachmentDescription colorAttachmentResolve =
					AttachmentDescription(swapchainFormat, vk::SampleCountFlagBits::e1,
						vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
						vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);



				vk::AttachmentReference refResolve = { 2,vk::ImageLayout::eColorAttachmentOptimal };
#else
				//Define a general attachment, with its load/store operations


				vk::AttachmentDescription colorAttachmentResolve =
					AttachmentDescription(swapchainFormat, vk::SampleCountFlagBits::e1,
						vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
						vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
				vk::AttachmentReference refResolve = { 2,vk::ImageLayout::eColorAttachmentOptimal };

#endif


				builder.AddAttachment({ colorAttachmentResolve , refResolve });
				

				//Renderpasses are broken down into subpasses, there's always at least one.

				builder.AddSubpass(SubpassDescription(1, &refMSAA, &refResolve, &refDepth));

				//vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0, 
				//	vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eNone,
				//	vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);

				vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);

				builder.AddSubpassDependency(dependency0);

				m_RenderPass = builder.Build(*device);

				
				{




					RenderPassBuilder builder;
					//Define a general attachment, with its load/store operations
					vk::AttachmentDescription msaaAttachment = AttachmentDescription(swapchainFormat,
						samples, vk::AttachmentLoadOp::eLoad,
						vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
						vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eColorAttachmentOptimal,
						vk::ImageLayout::eColorAttachmentOptimal);

					vk::AttachmentReference refMSAA = { 0,vk::ImageLayout::eColorAttachmentOptimal };
					builder.AddAttachment({ msaaAttachment ,refMSAA });

					vk::AttachmentDescription depthAttachment = AttachmentDescription(
						swapChainDepthFormat, samples, vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eLoad,
						vk::AttachmentStoreOp::eStore, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal);
					vk::AttachmentReference refDepth = { 1,vk::ImageLayout::eDepthStencilAttachmentOptimal };
					builder.AddAttachment({ depthAttachment , refDepth });



					vk::AttachmentDescription colorAttachmentResolve =
						AttachmentDescription(swapchainFormat, vk::SampleCountFlagBits::e1,
							vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
							vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eColorAttachmentOptimal);
					vk::AttachmentReference refResolve = { 2,vk::ImageLayout::eColorAttachmentOptimal };


					builder.AddAttachment({ colorAttachmentResolve ,refResolve });

					//Renderpasses are broken down into subpasses, there's always at least one.

					builder.AddSubpass(SubpassDescription(1, &refMSAA, &refResolve, &refDepth));

					vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
						vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite,
						vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);
					builder.AddSubpassDependency(dependency0);

					m_ClipRenderPass= builder.Build(*device);
				}


			}
			

			{
				RenderPassBuilder builder;

				//Define a general attachment, with its load/store operations
				vk::AttachmentDescription colorAttachment = AttachmentDescription(swapchainFormat,
					samples,vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
					vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
					vk::ImageLayout::eColorAttachmentOptimal);
				vk::AttachmentDescription colorAttachmentResolve =
					AttachmentDescription(swapchainFormat, vk::SampleCountFlagBits::e1,
						vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
						vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal);


				vk::AttachmentReference refColor = { 0,vk::ImageLayout::eColorAttachmentOptimal };
				vk::AttachmentReference refResolve = { 1,vk::ImageLayout::eColorAttachmentOptimal };
				builder.AddSubpass(SubpassDescription(1, &refColor,&refResolve));
				builder.AddAttachment({ colorAttachment,refColor });
				builder.AddAttachment({ colorAttachmentResolve,refResolve });
				vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eNone, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);
				builder.AddSubpassDependency(dependency0);
				m_PageRenderPass = builder.Build(*device);
			}
			{
				RenderPassBuilder builder;
				//Define a general attachment, with its load/store operations
				vk::AttachmentDescription msaaAttachment = AttachmentDescription(swapchainFormat,
					samples, vk::AttachmentLoadOp::eLoad,
					vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
					vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eColorAttachmentOptimal,
					vk::ImageLayout::eColorAttachmentOptimal);

				vk::AttachmentReference refMSAA = { 0,vk::ImageLayout::eColorAttachmentOptimal };
				builder.AddAttachment({ msaaAttachment ,refMSAA });

				vk::AttachmentDescription depthAttachment = AttachmentDescription(
					swapChainDepthFormat, samples, vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eLoad,
					vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal);
				vk::AttachmentReference refDepth = { 1,vk::ImageLayout::eDepthStencilAttachmentOptimal };
				builder.AddAttachment({ depthAttachment , refDepth });



				vk::AttachmentDescription colorAttachmentResolve =
					AttachmentDescription(swapchainFormat, vk::SampleCountFlagBits::e1,
						vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
						vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eColorAttachmentOptimal);
				vk::AttachmentReference refResolve = { 2,vk::ImageLayout::eColorAttachmentOptimal };


				builder.AddAttachment({ colorAttachmentResolve ,refResolve });

				//Renderpasses are broken down into subpasses, there's always at least one.

				builder.AddSubpass(SubpassDescription(1, &refMSAA, &refResolve, &refDepth));

				vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);
				builder.AddSubpassDependency(dependency0);

				m_NewPageRenderPass = builder.Build(*device);
				
			}

			{
				RenderPassBuilder builder;
				//Define a general attachment, with its load/store operations
				vk::AttachmentDescription msaaAttachment = AttachmentDescription(swapchainFormat,
					samples, vk::AttachmentLoadOp::eLoad,
					vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
					vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eColorAttachmentOptimal,
					vk::ImageLayout::eColorAttachmentOptimal);

				vk::AttachmentReference refMSAA = { 0,vk::ImageLayout::eColorAttachmentOptimal };
				builder.AddAttachment({ msaaAttachment ,refMSAA });

				vk::AttachmentDescription depthAttachment = AttachmentDescription(
					swapChainDepthFormat, samples, vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eLoad,
					vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal);
				vk::AttachmentReference refDepth = { 1,vk::ImageLayout::eDepthStencilAttachmentOptimal };
				builder.AddAttachment({ depthAttachment , refDepth });



				vk::AttachmentDescription colorAttachmentResolve =
					AttachmentDescription(swapchainFormat, vk::SampleCountFlagBits::e1,
						vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
						vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR);
				vk::AttachmentReference refResolve = { 2,vk::ImageLayout::eColorAttachmentOptimal };


				builder.AddAttachment({ colorAttachmentResolve ,refResolve });

				//Renderpasses are broken down into subpasses, there's always at least one.

				builder.AddSubpass(SubpassDescription(1, &refMSAA, &refResolve, &refDepth));

				vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);
				builder.AddSubpassDependency(dependency0);
				m_NewPageRenderPassRight = builder.Build(*device);
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


				




				auto samples = RenderContext::GetDevice()->GetSamples();
				
				auto pipelineLayouts = std::vector<vk::DescriptorSetLayout>{
					m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutFont->GetLayout() };



				PipelineBuilder builder;
				builder.SetDevice(device->GetDevice());
				builder.SetSamples(samples);
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
				builder.SetRenderPass(m_PageRenderPass);

				//builder.SetSampleShading(VK_TRUE);

				m_PagePipeline = builder.Build();

			
			}



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
				builder.AddExtent(swapChainExtent);
				builder.AddImageFormat(swapchainFormat);
				builder.EnableStencilTest(true);
				builder.SetDepthTest(true);
				builder.WriteToDepthBuffer(true);
				builder.SetRenderPass(m_RenderPass);
				builder.SetStencilRefNumber(2);
				builder.StencilTestOp(vk::CompareOp::eAlways,vk::StencilOp::eReplace, vk::StencilOp::eReplace, vk::StencilOp::eReplace);
				builder.SetMasks(0xff, 0xff);

				m_GraphicsPipeline = builder.Build();


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
					builder.SetSubpassAmount(0);
					builder.AddExtent(swapChainExtent);
					builder.AddImageFormat(swapchainFormat);
					builder.EnableStencilTest(true);
					builder.SetDepthTest(false);
					builder.WriteToDepthBuffer(true);
					builder.SetRenderPass(m_ClipRenderPass);
					builder.SetStencilRefNumber(2);
					builder.StencilTestOp(vk::CompareOp::eEqual, vk::StencilOp::eKeep, vk::StencilOp::eDecrementAndClamp, vk::StencilOp::eKeep);
					builder.SetMasks(0xff, 0xff);
					m_ClipPipeline = builder.Build();
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
						builder.SetSubpassAmount(0);
						builder.AddExtent(swapChainExtent);
						builder.AddImageFormat(swapchainFormat);
						builder.EnableStencilTest(true);
						builder.SetDepthTest(false);
						builder.WriteToDepthBuffer(true);
						builder.SetRenderPass(m_RenderPass);
						builder.SetStencilRefNumber(1);
						builder.StencilTestOp(vk::CompareOp::eEqual, vk::StencilOp::eKeep, vk::StencilOp::eReplace, vk::StencilOp::eKeep);
						builder.SetMasks(0xff, 0xff);
						m_NewPagePipeline = builder.Build();
						builder.DestroyShaderModules();
						builder.AddShader(BASE_SPIRV_OUTPUT + "default.spvV", vk::ShaderStageFlagBits::eVertex);
						Renderer::Instance()->CompileShader("rightNewPage.spvF", ShaderType::FRAGMENT);
						builder.AddShader(BASE_SPIRV_OUTPUT + "rightNewPage.spvF", vk::ShaderStageFlagBits::eFragment);
						builder.SetStencilRefNumber(2);
						builder.StencilTestOp(vk::CompareOp::eEqual, vk::StencilOp::eKeep, vk::StencilOp::eReplace, vk::StencilOp::eKeep);
						builder.SetBlendOp(vk::BlendOp::eAdd, vk::BlendFactor::eOne,vk::BlendFactor::eZero);
						m_NewPagePipelineRight = builder.Build();
					}
				}
				
				

			}


			vk::SemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.flags = vk::SemaphoreCreateFlags();

			try
			{
				m_RenderFinishedSemaphore1 = RenderContext::GetDevice()->GetDevice().createSemaphore(semaphoreInfo);
				m_RenderFinishedSemaphore2 = RenderContext::GetDevice()->GetDevice().createSemaphore(semaphoreInfo);
				m_RenderFinishedSemaphore3 = RenderContext::GetDevice()->GetDevice().createSemaphore(semaphoreInfo);
				m_RenderFinishedSemaphore4 = RenderContext::GetDevice()->GetDevice().createSemaphore(semaphoreInfo);
			}
			catch (vk::SystemError err) {

				Log::GetLog()->error("failed to create semaphore");
			}
					
		};

					

			

		
		auto submitRenderCommands = [&](size_t frameIndex,Camera& camera, vk::Semaphore& imageAvailable, vk::Fence& fence)
		{
			const auto scale = glm::vec3(glm::vec3(400, 512, 0));
			
#define LEFT_PAGE_COLOR whiteColor
#define RIGHT_PAGE_COLOR redColor
#define RIGHT_NEW_PAGE_COLOR blueColor
#define LEFT_NEW_PAGE_COLOR greyColor
			const float width = scale.x;
			const float height = scale.y;
			const float startPointX = 150*2.f;
			const float startPointY = 200 ;
			
			const glm::vec2 leftEdgeBottom = { startPointX  ,startPointY };
			const glm::vec2 spineTop = { startPointX + width ,startPointY + height };
			const glm::vec2 spineBottom = { startPointX + width,startPointY };
			const glm::vec2 center= { startPointX + width/2,startPointY + height /2};
			const glm::vec2 rightEdgeBottom = { width + startPointX + width,startPointY };
			const glm::vec2 rightEdgeTop = { width + startPointX + width,startPointY + height };
			

			UpdateUniformBuffer(frameIndex, camera);
			auto& renderCommandBuffer = Renderer::Instance()->GetRenderCommandBuffer(frameIndex);
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
						if (isDragged && glm::length(GetMousePosition(camera) - leftEdgeBottom) < 15*2)
						{
							std::swap(LEFT_PAGE_COLOR, LEFT_NEW_PAGE_COLOR);
							std::swap(textureIndicies[0], textureIndicies[2]);
							std::swap(RIGHT_PAGE_COLOR, RIGHT_NEW_PAGE_COLOR);
							std::swap(textureIndicies[1], textureIndicies[3]);
						}
						isClicked = false;
						isDragged = false;
						m_Follow = rightEdgeBottom;
						UpdateUniformBuffer(frameIndex,camera);
					}
					m_Follow = glm::clamp(m_Follow, leftEdgeBottom, glm::vec2{ rightEdgeBottom.x-0,center.y-40});

					glm::vec2 t0;
					t0.x = m_Follow.x + .5 * (rightEdgeBottom.x - m_Follow.x);
					t0.y = m_Follow.y + .5 * (rightEdgeBottom.y - m_Follow.y);
					t0 =  glm::clamp(t0, spineBottom, rightEdgeTop);
					auto bisectorAngle = glm::atan2(rightEdgeBottom.y - t0.y, rightEdgeBottom.x - t0.x);
					//t1
					auto bisectorTangent = t0.x -glm::tan(bisectorAngle) * (rightEdgeBottom.y - t0.y);
					//bisectorTangent = glm::max(bisectorTangent, spineBottom.x);
					if (bisectorTangent < spineBottom.x) bisectorTangent = spineBottom.x-2;
					if (bisectorTangent > rightEdgeBottom.x) bisectorTangent = rightEdgeBottom.x -2;
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
					auto deltaXPage= t1.x - m_Follow.x;
					auto deltaYPage= -t1.y + m_Follow.y;
					float pageAngle = glm::atan2( deltaXPage,deltaYPage);
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
					if (pageAngle > (3.14/2 ))
					{
						//pageAngle -= 3.14 ;
					}
					//std::cout << "mouse " << follow.x << " " << follow.y << "\n";
					//std::cout << "t0 " << t0.x << " " << t0.y << "\n";
					//std::cout << "t1 " << t1.x << " " << t1.y << "\n";
					std::vector<Vertex> leftPage{ 4 };
					leftPage[0].Position = { startPointX,startPointY,0 };
					leftPage[0].Color= LEFT_PAGE_COLOR;
					leftPage[1].Position = { startPointX,startPointY +height,0 };
					leftPage[1].Color= LEFT_PAGE_COLOR;
					leftPage[2].Position = { startPointX +width,startPointY,0 };
					leftPage[2].Color= LEFT_PAGE_COLOR;
					leftPage[3].Position = { startPointX +width,startPointY +height,0 };
					leftPage[3].Color= LEFT_PAGE_COLOR;
					leftPage[0].textureID= leftPage[1].textureID= leftPage[2].textureID = leftPage[3].textureID = textureIndicies[0];
					std::vector<Vertex> rightPage{ 4 };
					rightPage[0].Position = { width+startPointX,startPointY,0 };
					rightPage[0].Color = RIGHT_PAGE_COLOR;
					rightPage[1].Position = { width+startPointX,startPointY + height,0 };
					rightPage[1].Color = RIGHT_PAGE_COLOR;
					rightPage[2].Position = { 2*width+startPointX ,startPointY,0 };
					rightPage[2].Color = RIGHT_PAGE_COLOR;
					rightPage[3].Position = { glm::vec2{2*width + startPointX,startPointY + height},0 };
					rightPage[3].Color = RIGHT_PAGE_COLOR;
					rightPage[0].textureID= rightPage[1].textureID= rightPage[2].textureID = rightPage[3].textureID = textureIndicies[1];

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
					newLeftPage[0].textureID= newLeftPage[1].textureID= newLeftPage[2].textureID = newLeftPage[3].textureID = textureIndicies[2];

					std::vector<Vertex> newRightPage{ 4 };
					newRightPage[0].Position = { width + startPointX,startPointY,-0.5 };
					newRightPage[0].Color = RIGHT_NEW_PAGE_COLOR;
					newRightPage[1].Position = { width + startPointX,startPointY + height,-0.5 };
					newRightPage[1].Color = RIGHT_NEW_PAGE_COLOR;
					newRightPage[2].Position = { 2 * width + startPointX ,startPointY,-0.5 };
					newRightPage[2].Color = RIGHT_NEW_PAGE_COLOR;
					newRightPage[3].Position = { glm::vec2{2 * width + startPointX,startPointY + height},-0.5 };
					newRightPage[3].Color = RIGHT_NEW_PAGE_COLOR;
					newRightPage[0].textureID= newRightPage[1].textureID= newRightPage[2].textureID = newRightPage[3].textureID = textureIndicies[3];
					

					
					

					
					
					

					auto rotateVector = [](glm::vec3 v, float radians,glm::vec2 pointOfRotation) {
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
					float clipWidth  = 600;
					auto modifiedBlueColor = blueColor;
					modifiedBlueColor.a = 0;
					clipPage[0].Position = { t1.x - clipWidth  ,t1.y + clipHeight ,0 };
					clipPage[0].Color = modifiedBlueColor;
					clipPage[1].Position = { t1.x + clipWidth,t1.y + clipHeight ,0 };
					clipPage[1].Color = modifiedBlueColor;
					clipPage[2].Position = { t1.x - clipWidth   ,t1.y ,0 };
					clipPage[2].Color = modifiedBlueColor;
					clipPage[3].Position = { t1.x+ clipWidth,t1.y ,0 };
					clipPage[3].Color = modifiedBlueColor;

					for (int i = 0; i < 4; i++) {
						clipPage[i].Position = rotateVector(clipPage[i].Position, clipAngle, t1);
					}
					renderCommandBuffer.BeginRendering();

					auto swapchain = Renderer::Instance()->GetSwapchain();

					vk::CommandBufferBeginInfo beginInfo = {};

					auto device = RenderContext::GetDevice();
					auto viewportSize = Renderer::Instance()->GetViewportSize();
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

					auto render = [this,frameIndex,device,fence, viewport,scissors, swapchain,beginInfo](CommandBuffer& commandBuffer,
						std::vector<vk::ClearValue> clearValues, std::initializer_list<std::vector<Vertex>> list,
						vk::Semaphore renderFiniished, vk::Semaphore renderFiniishedSecond, vk::RenderPass& renderPass, Pipeline* pipeline)
					{
						device->GetDevice().waitForFences(fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
						device->GetDevice().resetFences(fence);
						Renderer::Instance()->BeginBatch();
						auto vkCommandBuffer = commandBuffer.GetCommandBuffer();
						vkCommandBuffer.begin(beginInfo);
						commandBuffer.BeginRenderPass(renderPass, swapchain.GetFrameBuffer(frameIndex), swapchain.GetExtent(), clearValues);

						vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 0, m_DescriptorSets[frameIndex], nullptr);
						vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 1, m_DescriptorSetTex, nullptr);
						vkCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());

						vkCommandBuffer.setViewport(0, 1, &viewport);
						vkCommandBuffer.setScissor(0, 1, &scissors);

						for (auto quad : list)
						{
							Renderer::Instance()->DrawQuad(quad);
						}

						Renderer::Instance()->DrawBatch(vkCommandBuffer);
						

						commandBuffer.EndRenderPass();
						vkCommandBuffer.end();
						{
							std::vector<vk::CommandBuffer> commandBuffers = { commandBuffer.GetCommandBuffer() };
							vk::Semaphore waitSemaphores[] = { renderFiniished };
							vk::Semaphore signalSemaphores[] = { renderFiniishedSecond };

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
							device->GetGraphicsQueue().submit(submitInfo, fence);
						}
						
					};
					vk::ClearValue clearColor = { std::array<float, 4>{137.f/255.f, 189.f / 255.f, 199.f / 255.f, 1.0f} };

					vk::ClearValue depthClear;
					uint32_t stencil0 = 3;
					depthClear.depthStencil = vk::ClearDepthStencilValue({ 1.0f, stencil0 });
					std::vector<vk::ClearValue> clearValues = { {clearColor, depthClear,clearColor} };
					std::vector<std::vector<Vertex>> pages =
					{
						leftPage,
						rightPage,
						newLeftPage,
						newRightPage
					};

				
					{
						

					
					
						render(renderCommandBuffer,  clearValues, { pages[0] ,pages[1]}, imageAvailable, m_RenderFinishedSemaphore1, m_RenderPass, m_GraphicsPipeline.get());


						render(renderCommandBuffer,  clearValues, { clipPage }, m_RenderFinishedSemaphore1, m_RenderFinishedSemaphore2, m_ClipRenderPass, m_ClipPipeline.get());
						render(renderCommandBuffer, clearValues, { pages[2]}, m_RenderFinishedSemaphore2, m_RenderFinishedSemaphore3, m_NewPageRenderPass, m_NewPagePipeline.get());
						render(renderCommandBuffer, clearValues, { pages[3]}, m_RenderFinishedSemaphore3, m_RenderFinishedSemaphore4, m_NewPageRenderPassRight, m_NewPagePipelineRight.get());
					
				

				}

				TracyVkCollect(ctx, commandBuffer);
				
				return m_RenderFinishedSemaphore4;
			}
		};
		auto postRenderCommands = [this](size_t frameIndex, Voidstar::Camera& camera)
		{
			if (Input::IsKeyTyped(VS_KEY_V))
			{
				// find mouse pos in world coordinates
				float scale = 100;
				auto  mousePos = Input::GetMousePos();
				auto mouseScreenPos = glm::vec2{ std::get<0>(mousePos),std::get<1>(mousePos) };
				auto viewportSize = Renderer::Instance()->GetViewportSize();
				glm::vec2 screenSize = { viewportSize.first, viewportSize.second };
				glm::vec3 ndc;
				ndc.x = (2.0f * mouseScreenPos.x / screenSize.x) - 1.0f;
				ndc.y = -(1.0f - (2.0f * mouseScreenPos.y / screenSize.y));

				auto vertPos = glm::vec4(0.5 * scale, 0, -0.5 * scale, 1);

				float planeDepthWorld = 0.934;
				float ndcZNear = 10;
				float ndcZFar = 10000;


				ndc.z = (planeDepthWorld - ndcZNear) / (ndcZFar - ndcZNear);
				auto inverseProj = glm::inverse(camera.GetProj());
				auto inverseView = glm::inverse(camera.GetView());
				auto rayEye = (inverseProj * glm::vec4{ ndc.x,ndc.y,1,1 });

				rayEye.z = 1;
				rayEye.w = 0;
				glm::vec3 rayDir = glm::normalize(glm::vec3(inverseView * rayEye));
				//	rayDir.z *= -1;
				glm::vec3 cameraPos = glm::vec3(inverseView[3]);
				std::cout << "ndc  " << ndc.x << " " << ndc.y << " " << ndc.z << std::endl;
				glm::vec3 rayOrigin = cameraPos;
				glm::vec3 intersectionPoint = { 0,0,0 };
				std::cout << "ray dir " << rayDir.x << " " << rayDir.y << " " << rayDir.z << std::endl;
				// works only  if camera doesn
				auto RayIntersectObjects = [](glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 planePoint, glm::vec3& intersectionPoint)
				{
					// Calculate the dot product of the ray direction and the plane's normal.
					glm::vec3 planeNormal = { 0,1,0 };
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

							std::cout << "intersection " << intersectionPoint.x << " " << intersectionPoint.y << " " << intersectionPoint.z << std::endl;
							return true; // Intersection occurred.
						}
					}
					return false;
				};

				bool hit = RayIntersectObjects(rayOrigin, rayDir, vertPos, intersectionPoint);
				/*worldSpace = worldSpace / worldSpace.w;

				*/
				auto localPosition = (intersectionPoint - glm::vec3(vertPos));
				glm::vec3 uAxis = glm::vec3(-1, 0, 0);
				glm::vec3 vAxis = glm::vec3(0, 0, -1);

				glm::vec2 uvSpace = glm::vec2{ glm::dot(glm::vec3{localPosition} , uAxis),glm::dot(glm::vec3{localPosition}, vAxis) } / 100.f;
				uvSpace.y *= -1;
				//uvSpace = glm::normalize(uvSpace);
				std::cout << "uv space " << uvSpace.x << " " << uvSpace.y << std::endl;
				//std::cout <<  "local pos " << localPosition.x << " " << localPosition.y << " " << localPosition.z << std::endl;
				//std::cout << "ray direction "  << rayDir.x << " " << rayDir.y << " " << rayDir.z << std::endl;
				// update vector of positions
				m_ClickPoints[nextPoint++] = (uvSpace);
				nextPoint %= MAX_POINTS;



				auto device = RenderContext::GetDevice();
				UpdateBuffer(*device, m_DescriptorSetSelected, m_ClickPoints.data(), *m_ShaderStorageBuffer, MAX_POINTS * sizeof(glm::vec2));



			}

			UpdateTexture(frameIndex);
		};


		// can be better
		auto createFramebuffer = [this]()
		{

			m_PagesImages.resize(PageAmount);
			m_PageFramebuffer.resize(PageAmount);
			m_PagesImageMSAA.resize(PageAmount);
			auto device = RenderContext::GetDevice();
			auto samples = device->GetSamples();
			for (int i = 0; i < m_PagesImages.size(); i++)
			{
				auto usage = vk::ImageUsageFlagBits::eColorAttachment  |vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
				m_PagesImages[i] = Image::CreateEmptyImage(PageRenderWidth, PageRenderHeight,vk::Format::eB8G8R8A8Unorm, usage, vk::SampleCountFlagBits::e1,vk::Filter::eLinear, vk::Filter::eLinear);
				
				
				ImageSpecs specs;
				auto extent = vk::Extent2D(PageRenderWidth, PageRenderHeight);
				auto swapchainFormat = vk::Format::eB8G8R8A8Unorm;
				specs.width = extent.width;
				specs.height = extent.height;
				specs.tiling = vk::ImageTiling::eOptimal;
				specs.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment;
				specs.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
				specs.format = swapchainFormat;
				auto samples = RenderContext::GetDevice()->GetSamples();
				auto msaaImage = Image::CreateVKImage(specs, samples);
				auto msaaImageMemory = Image::CreateMemory(msaaImage, specs);
				auto msaaImageView = Image::CreateImageView(msaaImage, swapchainFormat, vk::ImageAspectFlagBits::eColor);
				m_PagesImageMSAA[i] = CreateSPtr<Image>();
				m_PagesImageMSAA[i]->SetFormat(swapchainFormat);
				m_PagesImageMSAA[i]->SetView(msaaImageView);
				m_PagesImageMSAA[i]->SetMemory(msaaImageMemory);
				
				std::vector<vk::ImageView>attachments = { m_PagesImageMSAA[i]->GetImageView(),m_PagesImages[i]->GetImageView()};
				vk::FramebufferCreateInfo framebufferInfo;
				framebufferInfo.flags = vk::FramebufferCreateFlags();
				framebufferInfo.renderPass = m_PageRenderPass;
				framebufferInfo.attachmentCount = attachments.size();
				framebufferInfo.pAttachments = attachments.data();
				framebufferInfo.width = PageRenderWidth;
				framebufferInfo.height = PageRenderHeight;
				framebufferInfo.layers = 1;

				m_PageFramebuffer[i] = device->GetDevice().createFramebuffer(framebufferInfo);
			}

				
				
			

			auto& swapchain = Renderer::Instance()->GetSwapchain();
			auto& frames = swapchain.GetFrames();
			auto swapChainExtent = swapchain.GetExtent();
			auto msaaImageView = swapchain.GetMSAAImageView();
			for (int i = 0; i < frames.size(); ++i) {

				std::vector<vk::ImageView> attachments = {
					msaaImageView,
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
					frames[i].framebuffer = device->GetDevice().createFramebuffer(framebufferInfo);

				}
				catch (vk::SystemError err)
				{
					
					//Log::GetLog()->error("Failed to create framebuffer for frame{0} ", i);
					assert(false);
				}

			}
			{

				UniformBufferObject ubo{};
				auto camera = *GetCamera();
				auto cameraView = camera.GetView();
				auto cameraProj1 = camera.GetProj();
				auto orth = glm::ortho(0.f, PageRenderWidth, PageRenderHeight,0.f);
				auto cameraProj = orth;
				ubo.playerPos = glm::vec4{ camera.GetPosition(),0 };
				ubo.playerPos = glm::vec4{ m_Follow,0,0 };
				ubo.view = cameraView;
				ubo.proj = cameraProj;
				ubo.time = GetExeTime();
				memcpy(uniformBuffersMapped[0], &ubo, sizeof(ubo));

				auto& renderCommandBuffer = Renderer::Instance()->GetRenderCommandBuffer(0);
				auto& vkCommandBuffer = renderCommandBuffer.GetCommandBuffer();
				vk::CommandBufferBeginInfo beginInfo = {};
				
				vk::ClearValue clearColor = { std::array<float, 4>{0.f, 0.f, 0.f, 1.0f} };


				std::vector<vk::ClearValue> clearValues = { {clearColor,clearColor} };

				//glm::vec2 pos = { 20,20 };
				
				//str = R"(Mauris in euismod neque.\n
				//	Class aptent taciti sociosqu ad litora torquent per conubia nostra,\n
				//	per inceptos himenaeos. Curabitur id pellentesque ligula, quis eleifend dolor.\n
				//	Proin diam erat, vulputate eget rutrum cursus, placerat vitae massa. Fusce et sagittis odio. \n
				//	Mauris et ante mi. Morbi nisl odio, fringilla sit amet tortor ac,\n 
				//	fringilla maximus orci. Sed quis odio in enim egestas sollicitudin.\n 
				//	Cras aliquam nisl odio, egestas aliquet est hendrerit sed)";
//
//				str = R"(Mauris in euismod neque. 
//fringilla maximus orci.
//Sed quis odio in enim egestas sollicitudin.
//Cras aliquam nisl odio, egestas aliquet est hendrerit sed)";
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
				auto fence = Renderer::Instance()->GetFence();
				for (int i = 0; i <  m_PageFramebuffer.size(); i++)
				{
					device->GetDevice().waitForFences(fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
					device->GetDevice().resetFences(fence);
					vkCommandBuffer.begin(beginInfo);
					renderCommandBuffer.BeginRenderPass(m_PageRenderPass, m_PageFramebuffer[i], vk::Extent2D{static_cast<uint32_t>(PageRenderWidth),
					 static_cast<uint32_t>(PageRenderHeight)}, clearValues);
					glm::vec2 pos = { 20, 400};


					vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PagePipeline->GetLayout(), 0, m_DescriptorSets[0], nullptr);
					vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PagePipeline->GetLayout(), 1, m_DescriptorSetFont, nullptr);


					vkCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_PagePipeline->GetPipeline());

					vkCommandBuffer.setViewport(0, 1, &viewport);
					vkCommandBuffer.setScissor(0, 1, &scissors);
					Renderer::Instance()->BeginBatch();
					Renderer::Instance()->DrawTxt(vkCommandBuffer, txt[i% txt.size()], pos, Characters);
					size_t offset = 0;
					Renderer::Instance()->DrawBatch(vkCommandBuffer, offset);

					renderCommandBuffer.EndRenderPass();
					vkCommandBuffer.end();
					{
						std::vector<vk::CommandBuffer> commandBuffers = { vkCommandBuffer };

						vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
						vk::SubmitInfo submitInfo = {};

						//submitInfo.waitSemaphoreCount = 1;
						//submitInfo.pWaitSemaphores = waitSemaphores;
						submitInfo.pWaitDstStageMask = waitStages;


						submitInfo.commandBufferCount = commandBuffers.size();
						submitInfo.pCommandBuffers = commandBuffers.data();

						//submitInfo.signalSemaphoreCount = 1;
						//submitInfo.pSignalSemaphores = signalSemaphores;
						auto device = RenderContext::GetDevice();
						//device->GetDevice().resetFences(fence);
						device->GetGraphicsQueue().submit(submitInfo,fence);
						
					}

				}

				

				

				std::vector<vk::DescriptorImageInfo> infos(PageAmount);
				for (int i =0 ; i < PageAmount; i++)
				{
					infos[i].sampler = m_PagesImages[i]->GetSampler();
					infos[i].imageView= m_PagesImages[i]->GetImageView();
					infos[i].imageLayout= vk::ImageLayout::eShaderReadOnlyOptimal;

				}
				device->UpdateDescriptorSet(m_DescriptorSetTex, 0, infos, vk::DescriptorType::eCombinedImageSampler);

			}
		};

		auto cleanup = [this]()
		{

			

			auto device = RenderContext::GetDevice()->GetDevice();
	


			device.destroyRenderPass(m_RenderPass);
			device.destroyRenderPass(m_PageRenderPass);
			for (int i = 0; i < m_PageFramebuffer.size(); i++)
			{
				device.destroyFramebuffer(m_PageFramebuffer[i]);
			}
			for (int i = 0; i < m_PagesImages.size(); i++)
			{
				m_PagesImages[i].reset();
			}
			Renderer::Instance()->Shutdown();
			
		};
		Callables callables;
		callables.bindingsInit = bindingsInit;
		callables.bufferInit = bufferInit;
		callables.loadTextures = loadTextures;
		callables.bindResources = bindResources;
		callables.createPipelines = createPipelines;
		callables.createFramebuffer = createFramebuffer;
		callables.submitRenderCommands = submitRenderCommands;
		callables.postRenderCommands= postRenderCommands;
		callables.cleanUp= cleanup;


		Renderer::Instance()->SetCallables(callables);
		Renderer::Instance()->UserInit();
		
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

	void UpdateUniformBuffer(uint32_t imageIndex,Camera& camera)
	{
		UniformBufferObject ubo{};

		auto cameraView = camera.GetView();
		auto cameraProj = camera.GetProj();
		ubo.playerPos = glm::vec4{ camera.GetPosition(),0};
		ubo.playerPos = glm::vec4{ m_Follow,0,0};
		ubo.view = cameraView;
		ubo.proj = cameraProj;
		ubo.time = GetExeTime();
		memcpy(uniformBuffersMapped[imageIndex], &ubo, sizeof(ubo));

	}

	void UpdateTexture(size_t frameIndex)
	{
		auto device = RenderContext::GetDevice()->GetDevice();
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


		device.waitIdle();
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


private:

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
		
	std::vector<SPtr<Image>> m_PagesImages;
	std::vector<SPtr<Image>> m_PagesImageMSAA;
	std::vector<UPtr<Buffer>> m_UniformBuffers;
	std::vector<void*> uniformBuffersMapped;
	
	UPtr<Buffer> m_ShaderStorageBuffer;
	SPtr<Image> m_Image;
	SPtr<Image> m_ImageSelected;
	SPtr<Image> m_FontAtlas;
	std::vector<glm::vec2> m_ClickPoints;


	
	

	UPtr<Pipeline> m_ComputePipeline;
	UPtr<Pipeline> m_GraphicsPipeline;
	UPtr<Pipeline> m_ClipPipeline;
	UPtr<Pipeline> m_PagePipeline;
	UPtr<Pipeline> m_NewPagePipeline;
	UPtr<Pipeline> m_NewPagePipelineRight;

	vk::RenderPass m_RenderPass;
	vk::RenderPass m_ClipRenderPass;
	vk::RenderPass m_PageRenderPass;
	vk::RenderPass m_NewPageRenderPass;
	vk::RenderPass m_NewPageRenderPassRight;

	std::vector<vk::Framebuffer> m_PageFramebuffer;
	vk::Semaphore m_RenderFinishedSemaphore1;
	vk::Semaphore m_RenderFinishedSemaphore2;
	vk::Semaphore m_RenderFinishedSemaphore3;
	vk::Semaphore m_RenderFinishedSemaphore4;

	bool isClicked = false;
	bool isDragged = false;
	glm::vec4 whiteColor = { 0.2,0.7,0.2,1 }; // left
	glm::vec4 redColor = { 0.6,0,0,1 }; // right
	glm::vec4 greyColor = { 0.7,0.4,0.7,1 }; // new left
	glm::vec4 blueColor = { 0.2,0.2,0.7,1 }; // new right
	glm::vec2 m_Follow ;

	std::string PageStr1= "Hello, traveller!\nHow is it going?\nHave you found\nwhat you seek?\n\nAnyway...";
	std::string PageStr2 = "Here is page\ncurl rendering\n with Vulkan!\nThe text is rendered\nwith Vulkan too!";
	std::string PageStr3 = "This page rendering\nwas inspired by\nsimiliar effect in\n beloved\nDivinity: Original\n Sin Games";
	std::string PageStr4 = "May the force\nbe with you!\nHopefully this book\nenriched your\n potential!";
	std::vector<std::string> txt = { PageStr1,PageStr2,PageStr3,PageStr4 };
	std::vector<float> textureIndicies = { 0,1,2,3};



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
