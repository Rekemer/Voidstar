

#include <iostream>
#include <vector>
#include"Voidstar.h"
#include"tracy/Tracy.hpp"
#include"tracy/TracyVulkan.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H  
#include <map>
#include <utility>
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
				vk::AttachmentDescription msaaAttachment = AttachmentDescription(swapchainFormat, samples, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

				vk::AttachmentReference refMSAA = { 0,vk::ImageLayout::eColorAttachmentOptimal };
				builder.AddAttachment({ msaaAttachment ,refMSAA });

				vk::AttachmentDescription depthAttachment = AttachmentDescription(
					swapChainDepthFormat, samples, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
					vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
				vk::AttachmentReference refDepth = { 1,vk::ImageLayout::eDepthStencilAttachmentOptimal };
				builder.AddAttachment({ depthAttachment , refDepth });

				//Define a general attachment, with its load/store operations
				vk::AttachmentDescription colorAttachmentResolve =
					AttachmentDescription(swapchainFormat, vk::SampleCountFlagBits::e1,
						vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
						vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);



				vk::AttachmentReference refResolve = { 2,vk::ImageLayout::eColorAttachmentOptimal };
				builder.AddAttachment({ colorAttachmentResolve ,refResolve });

				//Renderpasses are broken down into subpasses, there's always at least one.

				builder.AddSubpass(SubpassDescription(1, &refMSAA, &refResolve, &refDepth));

				vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eNone, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);
				builder.AddSubpassDependency(dependency0);

				m_RenderPass = builder.Build(*device);
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

				GraphicsPipelineSpecification specs;

				specs.device = device->GetDevice();

				specs.vertexFilepath = BASE_SPIRV_OUTPUT + "font.spvV";
				specs.fragmentFilepath = BASE_SPIRV_OUTPUT + "font.spvF";
				specs.swapchainExtent = swapChainExtent;
				specs.swapchainImageFormat = swapchainFormat;


				std::vector<vk::VertexInputBindingDescription> bindings{ VertexBindingDescription(0,sizeof(Vertex),vk::VertexInputRate::eVertex) };

				std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;

				attributeDescriptions =
				{
					VertexInputAttributeDescription(0,0,vk::Format::eR32G32B32Sfloat,offsetof(Vertex, Position)),
					VertexInputAttributeDescription(0,1,vk::Format::eR32G32Sfloat,offsetof(Vertex, UV)),

				};

				specs.bindingDescription = bindings;



				specs.attributeDescription = attributeDescriptions;

				auto samples = RenderContext::GetDevice()->GetSamples();
				specs.samples = samples;
				
				auto pipelineLayouts = std::vector<vk::DescriptorSetLayout>{
					m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutFont->GetLayout() };

				specs.descriptorSetLayout = pipelineLayouts;


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
					//VertexInputAttributeDescription(0,1,vk::Format::eR32G32B32A32Sfloat,offsetof(Vertex, Color)),
					VertexInputAttributeDescription(0,1,vk::Format::eR32G32Sfloat,offsetof(Vertex, UV)),

				};

				specs.bindingDescription = bindings;



				specs.attributeDescription = attributeDescriptions;

				auto samples = RenderContext::GetDevice()->GetSamples();
				specs.samples = samples;
				auto pipelineLayouts = std::vector<vk::DescriptorSetLayout>{ m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutFont->GetLayout() };

				specs.descriptorSetLayout = pipelineLayouts;




				PipelineBuilder builder;
				builder.SetDevice(device->GetDevice());
				builder.SetSamples(samples);
				builder.AddDescriptorLayouts(pipelineLayouts);
				builder.AddAttributeDescription(attributeDescriptions);
				builder.AddBindingDescription(bindings);
				builder.SetPolygoneMode(Renderer::Instance()->GetPolygonMode());
				builder.SetTopology(vk::PrimitiveTopology::eTriangleList);
				builder.AddShader(BASE_SPIRV_OUTPUT + "font.spvV", vk::ShaderStageFlagBits::eVertex);
				builder.AddShader(BASE_SPIRV_OUTPUT + "font.spvF", vk::ShaderStageFlagBits::eFragment);
				builder.SetSubpassAmount(0);
				builder.AddExtent(swapChainExtent);
				builder.AddImageFormat(swapchainFormat);
				builder.SetRenderPass(m_RenderPass);
				m_GraphicsPipeline = builder.Build();

			}
			UpdateTexture(0);
		};

		auto submitRenderCommands = [this](size_t frameIndex,Camera& camera)
		{

			UpdateUniformBuffer(frameIndex, camera);
			auto& renderCommandBuffer = Renderer::Instance()->GetRenderCommandBuffer(frameIndex);
			{
				ZoneScopedN("Sumbit render commands");
				renderCommandBuffer.BeginRendering();

				auto swapchain = Renderer::Instance()->GetSwapchain();

				vk::CommandBufferBeginInfo beginInfo = {};

				auto commandBuffer = renderCommandBuffer.GetCommandBuffer();

				//RenderContext::GetDevice()->GetDevice().waitIdle();
				commandBuffer.begin(beginInfo);
		/*		{



					TracyVkZone(ctx, commandBuffer, "Rendering ");



					vk::ClearValue clearColor = { std::array<float, 4>{0.1f, .3f, 0.1f, 1.0f} };

					vk::ClearValue depthClear;

					depthClear.depthStencil = vk::ClearDepthStencilValue({ 1.0f, 0 });
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

					Renderer:: Instance()->DrawQuadIndexed(commandBuffer);

					renderCommandBuffer.EndRenderPass();


					
					


				}*/
				{
					vk::ClearValue clearColor = { std::array<float, 4>{0.1f, .3f, 0.1f, 1.0f} };

					vk::ClearValue depthClear;

					depthClear.depthStencil = vk::ClearDepthStencilValue({ 1.0f, 0 });
					std::vector<vk::ClearValue> clearValues = { {clearColor, depthClear,clearColor} };
					renderCommandBuffer.BeginRenderPass(m_RenderPass, swapchain.GetFrameBuffer(frameIndex), swapchain.GetExtent(), clearValues);
					glm::vec2 pos = { 100,500 };
					std::string str = "Hello\n  sailor!";
				//str = R"(Mauris in euismod neque.\n
				//	Class aptent taciti sociosqu ad litora torquent per conubia nostra,\n
				//	per inceptos himenaeos. Curabitur id pellentesque ligula, quis eleifend dolor.\n
				//	Proin diam erat, vulputate eget rutrum cursus, placerat vitae massa. Fusce et sagittis odio. \n
				//	Mauris et ante mi. Morbi nisl odio, fringilla sit amet tortor ac,\n 
				//	fringilla maximus orci. Sed quis odio in enim egestas sollicitudin.\n 
				//	Cras aliquam nisl odio, egestas aliquet est hendrerit sed)";

					str = R"(Mauris in euismod neque. 
	fringilla maximus orci.
	Sed quis odio in enim egestas sollicitudin.
	Cras aliquam nisl odio, egestas aliquet est hendrerit sed)";

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
					commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline->GetLayout(), 1, m_DescriptorSetFont, nullptr);


					commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline->GetPipeline());

					commandBuffer.setViewport(0, 1, &viewport);
					commandBuffer.setScissor(0, 1, &scissors);
					Renderer::Instance()->BeginBatch();
					Renderer::Instance()->DrawTxt(commandBuffer,str,pos,Characters);
				//	Renderer::Instance()->DrawTxt(commandBuffer,"I love you!" ,2.f*pos, Characters);

					renderCommandBuffer.EndRenderPass();
				}
				TracyVkCollect(ctx, commandBuffer);
				commandBuffer.end();
				renderCommandBuffer.EndRendering();
				return renderCommandBuffer;
			}
		};
		auto postRenderCommands = [this](size_t frameIndex, Voidstar::Camera& camera)
		{
			if (Input::IsKeyTyped(VS_KEY_V))
			{
				// find mouse pos in world coordinates
				float scale = 100;
				auto  mousePos = Input::GetMousePos();
				auto viewportSize = Renderer::Instance()->GetViewportSize();
				auto mouseScreenPos = glm::vec2{ std::get<0>(mousePos),std::get<1>(mousePos) };
				glm::vec3 ndc;
				glm::vec2 screenSize = { viewportSize.first, viewportSize.second };
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
	UPtr<Pipeline> m_PagePipeline;

	vk::RenderPass m_RenderPass;

	vk::Framebuffer m_PageFramebuffer;
	vk::RenderPass m_PageRenderPass;
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
