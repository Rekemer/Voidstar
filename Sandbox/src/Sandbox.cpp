

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
			binderRender.Bind(0, 1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
			binderRender.Bind(1, 1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);

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
			device->UpdateDescriptorSet(m_DescriptorSetTex, 0, 1, *m_ImageSelected, vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eCombinedImageSampler);
			device->UpdateDescriptorSet(m_DescriptorSetTex, 1, 1, *m_Image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eCombinedImageSampler);


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
				vk::AttachmentDescription colorAttachment = AttachmentDescription(swapchainFormat,vk::SampleCountFlagBits::e1,vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

				vk::AttachmentReference refColor = { 0,vk::ImageLayout::eColorAttachmentOptimal };
				builder.AddSubpass(SubpassDescription(1, &refColor));
				builder.AddAttachment({ colorAttachment,refColor });
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
						vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR);
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


				std::vector<vk::VertexInputBindingDescription> bindings{ VertexBindingDescription(0,sizeof(Vertex),vk::VertexInputRate::eVertex) };


				std::vector<vk::VertexInputAttributeDescription>
					attributeDescriptions =
				{
					VertexInputAttributeDescription(0,0,vk::Format::eR32G32B32Sfloat,offsetof(Vertex, Position)),
					VertexInputAttributeDescription(0,1,vk::Format::eR32G32Sfloat,offsetof(Vertex, UV)),

				};





				auto samples = RenderContext::GetDevice()->GetSamples();
				
				auto pipelineLayouts = std::vector<vk::DescriptorSetLayout>{
					m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutFont->GetLayout() };



				PipelineBuilder builder;
				builder.SetDevice(device->GetDevice());
				builder.SetSamples(vk::SampleCountFlagBits::e1);
				builder.AddDescriptorLayouts(pipelineLayouts);
				builder.AddAttributeDescription(attributeDescriptions);
					builder.AddBindingDescription(bindings);
				builder.SetPolygoneMode(Renderer::Instance()->GetPolygonMode());
				builder.SetTopology(vk::PrimitiveTopology::eTriangleList);
				builder.AddShader(BASE_SPIRV_OUTPUT + "font.spvV", vk::ShaderStageFlagBits::eVertex);
				builder.AddShader(BASE_SPIRV_OUTPUT + "font.spvF", vk::ShaderStageFlagBits::eFragment);
				builder.SetSubpassAmount(0);
				
				builder.AddExtent(vk::Extent2D{ 128 ,128 });
				builder.AddImageFormat(vk::Format::eB8G8R8A8Unorm);
				builder.SetRenderPass(m_PageRenderPass);



				//m_PagePipeline = Pipeline::CreateGraphicsPipeline(specs, vk::PrimitiveTopology::eTriangleList, swapchain.GetDepthFormat(), m_PageRenderPass, 0, false, Renderer::Instance()->GetPolygonMode());
				m_PagePipeline = builder.Build();

			
			}



			{
				GraphicsPipelineSpecification specs;

				specs.device = device->GetDevice();

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




				PipelineBuilder builder;
				builder.SetDevice(device->GetDevice());
				builder.SetSamples(samples);
				builder.AddDescriptorLayouts(pipelineLayouts);
				builder.AddAttributeDescription(attributeDescriptions);
				builder.AddBindingDescription(bindings);
				builder.SetPolygoneMode(Renderer::Instance()->GetPolygonMode());
				builder.SetTopology(vk::PrimitiveTopology::eTriangleList);
				builder.AddShader(BASE_SPIRV_OUTPUT + "default.spvV", vk::ShaderStageFlagBits::eVertex);
				builder.AddShader(BASE_SPIRV_OUTPUT + "dos2.spvF", vk::ShaderStageFlagBits::eFragment);
				builder.SetSubpassAmount(0);
				builder.AddExtent(swapChainExtent);
				builder.AddImageFormat(swapchainFormat);
				builder.EnableStencilTest(true);
				builder.SetDepthTest(true);
				builder.WriteToDepthBuffer(true);
				builder.SetRenderPass(m_RenderPass);
				builder.SetStencilRefNumber(1);
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
					builder.AddShader(BASE_SPIRV_OUTPUT + "dos2.spvF", vk::ShaderStageFlagBits::eFragment);
					builder.SetSubpassAmount(0);
					builder.AddExtent(swapChainExtent);
					builder.AddImageFormat(swapchainFormat);
					builder.EnableStencilTest(true);
					builder.SetDepthTest(false);
					builder.WriteToDepthBuffer(true);
					builder.SetRenderPass(m_ClipRenderPass);
					builder.SetStencilRefNumber(0);
					builder.StencilTestOp(vk::CompareOp::eLess, vk::StencilOp::eKeep, vk::StencilOp::eReplace, vk::StencilOp::eKeep);
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
						builder.AddShader(BASE_SPIRV_OUTPUT + "default.spvV", vk::ShaderStageFlagBits::eVertex);
						builder.AddShader(BASE_SPIRV_OUTPUT + "dos2.spvF", vk::ShaderStageFlagBits::eFragment);
						builder.SetSubpassAmount(0);
						builder.AddExtent(swapChainExtent);
						builder.AddImageFormat(swapchainFormat);
						builder.EnableStencilTest(true);
						builder.SetDepthTest(false);
						builder.WriteToDepthBuffer(true);
						builder.SetRenderPass(m_RenderPass);
						builder.SetStencilRefNumber(0);
						builder.StencilTestOp(vk::CompareOp::eEqual, vk::StencilOp::eKeep, vk::StencilOp::eReplace, vk::StencilOp::eKeep);
						builder.SetMasks(0xff, 0xff);
						m_NewPagePipeline = builder.Build();
					}
				}
				

			}
			UpdateTexture(0);


			vk::SemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.flags = vk::SemaphoreCreateFlags();

			try
			{
				m_RenderFinishedSemaphore1 = RenderContext::GetDevice()->GetDevice().createSemaphore(semaphoreInfo);
				m_RenderFinishedSemaphore2 = RenderContext::GetDevice()->GetDevice().createSemaphore(semaphoreInfo);
				m_RenderFinishedSemaphore3 = RenderContext::GetDevice()->GetDevice().createSemaphore(semaphoreInfo);
			}
			catch (vk::SystemError err) {

				Log::GetLog()->error("failed to create semaphore");
			}

		};


		enum class SideTurning
		{
			LEFT,
			RIGHT
		};

		
		auto submitRenderCommands = [this](size_t frameIndex,Camera& camera, vk::Semaphore& imageAvailable, vk::Fence& fence)
		{
			const auto scale = glm::vec3(glm::vec3(250, 300, 0));
			const glm::vec4 whiteColor = { 1,1,1,1 };
			const glm::vec4 redColor = { 1,0,0,1 };
			const glm::vec4 greyColor = { 0.7,0.7,0.7,1 };
			const glm::vec4 blueColor = { 0.2,0.2,0.7,0.5 };
			const float width = scale.x;
			const float height = scale.y;
			const float startPointX = 150;
			const float startPointY = 200;

			const glm::vec2 leftEdgeBottom = { startPointX  ,startPointY };
			const glm::vec2 spineTop = { startPointX + width ,startPointY + height };
			const glm::vec2 spineBottom = { startPointX + width,startPointY };
			const glm::vec2 rightEdgeBottom = { width + startPointX + width,startPointY };
			const glm::vec2 rightEdgeTop = { width + startPointX + width,startPointY + height };
			if (Input::IsMousePressed(0))
			{

			}

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

					
					auto follow = GetMousePosition(camera);
					follow = glm::clamp(follow, leftEdgeBottom, rightEdgeTop);

					glm::vec2 t0;
					t0.x = follow.x + .5 * (rightEdgeBottom.x - follow.x);
					t0.y = follow.y + .5 * (rightEdgeBottom.y - follow.y);
					t0 =  glm::clamp(t0, spineBottom, rightEdgeTop);
					auto bisectorAngle = glm::atan2(rightEdgeBottom.y - t0.y, rightEdgeBottom.x - t0.x);
					//t1
					auto bisectorTangent = t0.x -glm::tan(bisectorAngle) * (rightEdgeBottom.y - t0.y);
					//bisectorTangent = glm::max(bisectorTangent, spineBottom.x);
					if (bisectorTangent < spineBottom.x) bisectorTangent = spineBottom.x;
					if (bisectorTangent > rightEdgeBottom.x) bisectorTangent = rightEdgeBottom.x;
					glm::vec2 t1;
					glm::vec2 t2;
					t1.x = bisectorTangent;
					t1.y = rightEdgeBottom.y;
					t2.x = t0.x;
					t2.y = t1.y;
					const auto PI2 = 3.14f / 2.f;
					const auto PI = 3.14f;
					auto deltaXPage= t1.x - follow.x;
					auto deltaYPage= -t1.y + follow.y;
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
					t1 = glm::clamp(t1, spineBottom, rightEdgeBottom);
					t2 = glm::clamp(t2, spineBottom, rightEdgeBottom);
					pageAngle = glm::clamp(pageAngle, 0.f, PI);

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
					leftPage[0].Color= whiteColor;
					leftPage[1].Position = { startPointX,startPointY +height,0 };
					leftPage[1].Color= whiteColor;
					leftPage[2].Position = { startPointX +width,startPointY,0 };
					leftPage[2].Color= whiteColor;
					leftPage[3].Position = { startPointX +width,startPointY +height,0 };
					leftPage[3].Color= whiteColor;
					std::vector<Vertex> rightPage{ 4 };
					rightPage[0].Position = { width+startPointX,startPointY,0 };
					rightPage[0].Color = redColor;
					rightPage[1].Position = { width+startPointX,startPointY + height,0 };
					rightPage[1].Color = redColor;
					rightPage[2].Position = { 2*width+startPointX ,startPointY,0 };
					rightPage[2].Color = redColor;
					rightPage[3].Position = { glm::vec2{2*width + startPointX,startPointY + height},0 };
					rightPage[3].Color = redColor;

					std::vector<Vertex> newLeftPage{ 4 };
					//newLeftPage[0].Position = { 2 * width + startPointX,startPointY,0 };
					newLeftPage[0].Position = { follow,0 };
					newLeftPage[0].Color = greyColor;
					newLeftPage[1].Position = { newLeftPage[0].Position.x,newLeftPage[0].Position.y - height,0 };
					newLeftPage[1].Color = greyColor;
					newLeftPage[2].Position = { newLeftPage[0].Position.x - width ,newLeftPage[0].Position.y,0 };
					newLeftPage[2].Color = greyColor;
					newLeftPage[3].Position = { newLeftPage[0].Position.x - width,newLeftPage[0].Position.y - height ,0 };
					newLeftPage[3].Color = greyColor;


					

					
					

					
					
					

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
					
					float clipHeight = 300;
					float clipWidth  = 350;
					clipPage[0].Position = { t1.x - clipWidth  ,t1.y + clipHeight ,0 };
					clipPage[0].Color = blueColor;
					clipPage[1].Position = { t1.x + clipWidth,t1.y + clipHeight ,0 };
					clipPage[1].Color = blueColor;
					clipPage[2].Position = { t1.x - clipWidth   ,t1.y ,0 };
					clipPage[2].Color = blueColor;
					clipPage[3].Position = { t1.x+ clipWidth,t1.y ,0 };
					clipPage[3].Color = blueColor;

					for (int i = 0; i < 4; i++) {
						clipPage[i].Position = rotateVector(clipPage[i].Position, clipAngle, t1);
					}
					renderCommandBuffer.BeginRendering();

					auto swapchain = Renderer::Instance()->GetSwapchain();

					vk::CommandBufferBeginInfo beginInfo = {};

					auto commandBuffer = renderCommandBuffer.GetCommandBuffer();

					//RenderContext::GetDevice()->GetDevice().waitIdle();
					{

						commandBuffer.begin(beginInfo);


						TracyVkZone(ctx, commandBuffer, "Rendering ");



						vk::ClearValue clearColor = { std::array<float, 4>{0.1f, .3f, 0.1f, 1.0f} };

						vk::ClearValue depthClear;

						uint32_t stencil0 = 2;
						depthClear.depthStencil = vk::ClearDepthStencilValue({ 1.0f, stencil0 });
						std::vector<vk::ClearValue> clearValues = { {clearColor, depthClear,clearColor} };
						renderCommandBuffer.BeginRenderPass(m_RenderPass, swapchain.GetFrameBuffer(frameIndex), swapchain.GetExtent(), clearValues);

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

						commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline->GetLayout(), 0, m_DescriptorSets[frameIndex], nullptr);
						commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline->GetLayout(), 1, m_DescriptorSetTex, nullptr);


						commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline->GetPipeline());

						commandBuffer.setViewport(0, 1, &viewport);
						commandBuffer.setScissor(0, 1, &scissors);


						Renderer::Instance()->BeginBatch();
					Renderer:: Instance()->DrawQuad(leftPage);
					Renderer:: Instance()->DrawQuad(rightPage);
					
					//Renderer:: Instance()->DrawQuad(clipPage);
					auto identity = glm::identity<glm::mat4>();
					auto debug0 = glm::translate(identity, glm::vec3(t0, 0));
					auto debug1 = glm::translate(identity, glm::vec3(t1, 0));
					auto debug2 = glm::translate(identity, glm::vec3(t2, 0));
					debug0 = glm::scale(debug0, glm::vec3(10, 10, 0));
					debug1 = glm::scale(debug1, glm::vec3(10, 10, 0));
					debug2 = glm::scale(debug2, glm::vec3(10, 10, 0));
					auto debug3 = glm::translate(identity, glm::vec3(clipPage[0].Position));
					debug3 = glm::scale(debug3, glm::vec3(10, 10, 0));
					Renderer:: Instance()->DrawQuad(debug0, glm::vec4(1, 0, 1, 1));
					Renderer:: Instance()->DrawQuad(debug1, glm::vec4(0, 1, 1, 1));
					Renderer:: Instance()->DrawQuad(debug2, glm::vec4(1, 1, 1, 1));
					Renderer:: Instance()->DrawQuad(debug3, glm::vec4(0.3, 0, 0, 1));
					//Renderer:: Instance()->DrawQuad( world2, glm::vec4(0, 0, 1, 1));
					Renderer:: Instance()->DrawBatch(commandBuffer);
					renderCommandBuffer.EndRenderPass();


					commandBuffer.end();
					
					auto device = RenderContext::GetDevice();
					{
						std::vector<vk::CommandBuffer> commandBuffers = { renderCommandBuffer.GetCommandBuffer() };
						vk::Semaphore waitSemaphores[] = { imageAvailable };
						vk::Semaphore signalSemaphores[] = { m_RenderFinishedSemaphore1 };

						vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
						vk::SubmitInfo submitInfo = {};

						submitInfo.waitSemaphoreCount = 1;
						submitInfo.pWaitSemaphores = waitSemaphores;
						submitInfo.pWaitDstStageMask = waitStages;


						submitInfo.commandBufferCount = commandBuffers.size();
						submitInfo.pCommandBuffers = commandBuffers.data();

						submitInfo.signalSemaphoreCount = 1;
						submitInfo.pSignalSemaphores = signalSemaphores;
						
						device->GetGraphicsQueue().submit(submitInfo, fence);
					}
					device->GetDevice().waitForFences(fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
					device->GetDevice().resetFences(fence);

						Renderer::Instance()->BeginBatch();
						commandBuffer.begin(beginInfo);
						renderCommandBuffer.BeginRenderPass(m_ClipRenderPass, swapchain.GetFrameBuffer(frameIndex), swapchain.GetExtent(), clearValues);

						commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_ClipPipeline->GetLayout(), 0, m_DescriptorSets[frameIndex], nullptr);
						commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_ClipPipeline->GetLayout(), 1, m_DescriptorSetTex, nullptr);
						commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_ClipPipeline->GetPipeline());

						commandBuffer.setViewport(0, 1, &viewport);
						commandBuffer.setScissor(0, 1, &scissors);

						Renderer::Instance()->DrawQuad(clipPage);
						Renderer::Instance()->DrawBatch(commandBuffer);
						
					renderCommandBuffer.EndRenderPass();
						commandBuffer.end();
					{
							std::vector<vk::CommandBuffer> commandBuffers = { renderCommandBuffer.GetCommandBuffer() };
							vk::Semaphore waitSemaphores[] = { m_RenderFinishedSemaphore1 };
							vk::Semaphore signalSemaphores[] = { m_RenderFinishedSemaphore2 };

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
					device->GetDevice().waitForFences(fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
					device->GetDevice().resetFences(fence);
					Renderer::Instance()->BeginBatch();
					commandBuffer.begin(beginInfo);

					renderCommandBuffer.BeginRenderPass(m_NewPageRenderPass, swapchain.GetFrameBuffer(frameIndex), swapchain.GetExtent(), clearValues);
					commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_NewPagePipeline->GetLayout(), 0, m_DescriptorSets[frameIndex], nullptr);
					commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_NewPagePipeline->GetLayout(), 1, m_DescriptorSetTex, nullptr);
					commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_NewPagePipeline->GetPipeline());

					commandBuffer.setViewport(0, 1, &viewport);
					commandBuffer.setScissor(0, 1, &scissors);

					Renderer::Instance()->DrawQuad(newLeftPage);
					Renderer::Instance()->DrawBatch(commandBuffer);
					renderCommandBuffer.EndRenderPass();
					commandBuffer.end();
					{
						std::vector<vk::CommandBuffer> commandBuffers = { renderCommandBuffer.GetCommandBuffer() };
						vk::Semaphore waitSemaphores[] = { m_RenderFinishedSemaphore2 };
						vk::Semaphore signalSemaphores[] = { m_RenderFinishedSemaphore3 };

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
					
				

				}
//				{
//					vk::ClearValue clearColor = { std::array<float, 4>{0.1f, .3f, 0.1f, 1.0f} };
//
//					vk::ClearValue depthClear;
//
//					depthClear.depthStencil = vk::ClearDepthStencilValue({ 1.0f, 0 });
//					std::vector<vk::ClearValue> clearValues = { {clearColor, depthClear,clearColor} };
//					renderCommandBuffer.BeginRenderPass(m_PageRenderPass, m_PageFramebuffer, vk::Extent2D{128,128}, clearValues);
//					glm::vec2 pos = { 100,500 };
//					std::string str = "Hello\n  sailor!";
//				//str = R"(Mauris in euismod neque.\n
//				//	Class aptent taciti sociosqu ad litora torquent per conubia nostra,\n
//				//	per inceptos himenaeos. Curabitur id pellentesque ligula, quis eleifend dolor.\n
//				//	Proin diam erat, vulputate eget rutrum cursus, placerat vitae massa. Fusce et sagittis odio. \n
//				//	Mauris et ante mi. Morbi nisl odio, fringilla sit amet tortor ac,\n 
//				//	fringilla maximus orci. Sed quis odio in enim egestas sollicitudin.\n 
//				//	Cras aliquam nisl odio, egestas aliquet est hendrerit sed)";
//
//					str = R"(Mauris in euismod neque. 
//fringilla maximus orci.
//Sed quis odio in enim egestas sollicitudin.
//Cras aliquam nisl odio, egestas aliquet est hendrerit sed)";
//
//					auto viewportSize = Renderer::Instance()->GetViewportSize();
//					vk::Viewport viewport;
//					viewport.x = 0;
//					viewport.y = 0;
//					viewport.minDepth = 0;
//					viewport.maxDepth = 1;
//					viewport.height = viewportSize.second;
//					viewport.width = viewportSize.first;
//
//					vk::Rect2D scissors;
//					scissors.offset = vk::Offset2D{ (uint32_t)0,(uint32_t)0 };
//					scissors.extent = vk::Extent2D{ (uint32_t)viewportSize.first,(uint32_t)viewportSize.second };
//
//
//
//
//
//					commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PagePipeline->GetLayout(), 0, m_DescriptorSets[frameIndex], nullptr);
//					commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PagePipeline->GetLayout(), 1, m_DescriptorSetFont, nullptr);
//
//
//					commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_PagePipeline->GetPipeline());
//
//					commandBuffer.setViewport(0, 1, &viewport);
//					commandBuffer.setScissor(0, 1, &scissors);
//					Renderer::Instance()->BeginBatch();
//					Renderer::Instance()->DrawTxt(commandBuffer,str,pos,Characters);
//				//	Renderer::Instance()->DrawTxt(commandBuffer,"I love you!" ,2.f*pos, Characters);
//
//					renderCommandBuffer.EndRenderPass();
//				}
				TracyVkCollect(ctx, commandBuffer);
				
				return m_RenderFinishedSemaphore3;
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

			m_PagesImages.resize(4);
			for (int i = 0; i < m_PagesImages.size(); i++)
			{
				auto usage = vk::ImageUsageFlagBits::eColorAttachment  |vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
				m_PagesImages[i] = Image::CreateEmptyImage(128,128,vk::Format::eB8G8R8A8Unorm, usage);
			}

			std::vector<vk::ImageView> attachments = {
				
					m_PagesImages[0]->GetImageView(),
			};
			vk::FramebufferCreateInfo framebufferInfo;
			framebufferInfo.flags = vk::FramebufferCreateFlags();
			framebufferInfo.renderPass = m_PageRenderPass;
			framebufferInfo.attachmentCount = attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = 128;
			framebufferInfo.height = 128;
			framebufferInfo.layers = 1;

			auto device = RenderContext::GetDevice();
			m_PageFramebuffer = device->GetDevice().createFramebuffer(framebufferInfo);

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
		};

		auto cleanup = [this]()
		{

			

			auto device = RenderContext::GetDevice()->GetDevice();
	


			device.destroyRenderPass(m_RenderPass);
			device.destroyRenderPass(m_PageRenderPass);
			device.destroyFramebuffer(m_PageFramebuffer);
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
		//glm::vec3 eye = { 1.0f, 0.0f, 5.0f };
		//glm::vec3 center = { 0.5f, -1.0f, 0.0f };
		//glm::vec3 up = { 0.0f, 0.0f, 1.0f };
		//cameraView = glm::lookAt(eye,center,up);
		//ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//ubo.view = glm::lookAt(glm::vec3(-2.0f, 0.0f, 4.0f), glm::vec3(0.5f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		////ubo.proj = glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.0001f, 10.0f);
		ubo.playerPos = glm::vec4{ camera.GetPosition(),0};
		ubo.view = cameraView;
		ubo.proj = cameraProj;
		ubo.time = GetExeTime();
		//auto model = glm::mat4(1.f);
		//glm::mat4 blenderToLH = glm::mat4(1.0f);
		//blenderToLH[2][2] = -1.0f;  // Flip Z-axis
		//blenderToLH[3][2] = 1.0f;
		////model = blenderToLH * model;
		//// blender: z  is up, y is forward
		//model = glm::rotate(model,glm::radians(-90.f) , glm::vec3(1, 0, 0));
		//model = glm::rotate(model,glm::radians(90.f) , glm::vec3(0, 0, 1));
		//ubo.model = model;
		//ubo.proj[1][1] *= -1,
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
		auto error = FT_Set_Pixel_Sizes(face, 0, 48);



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
			if (FT_Load_Char(face, c, FT_LOAD_RENDER) != 0)
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

	vk::RenderPass m_RenderPass;
	vk::RenderPass m_ClipRenderPass;
	vk::RenderPass m_PageRenderPass;
	vk::RenderPass m_NewPageRenderPass;

	vk::Framebuffer m_PageFramebuffer;
	vk::Semaphore m_RenderFinishedSemaphore1;
	vk::Semaphore m_RenderFinishedSemaphore2;
	vk::Semaphore m_RenderFinishedSemaphore3;

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
