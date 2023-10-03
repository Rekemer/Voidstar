

#include <iostream>
#include <vector>
#include"Voidstar.h"
#include"tracy/Tracy.hpp"
#include"tracy/TracyVulkan.hpp"
using namespace Voidstar;


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
				m_UniformBuffers[i] = new Buffer(inputBuffer);
				uniformBuffersMapped[i] = m_Device->GetDevice().mapMemory(m_UniformBuffers[i]->GetMemory(), 0, bufferSize);
			}



			std::vector<IndexType> indices;
			auto vertices = GeneratePlane(1, indices);
			auto indexSize = SizeOfBuffer(indices.size(), indices[0]);
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
				BufferInputChunk info;
				info.size = sizeof(glm::vec2) * MAX_POINTS;
				info.usage = vk::BufferUsageFlagBits::eStorageBuffer |
					vk::BufferUsageFlagBits::eTransferDst;
				info.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible |
					vk::MemoryPropertyFlagBits::eHostCoherent;
				m_ShaderStorageBuffer = CreateUPtr<Buffer>(info);
			}
		};



		auto commandBufferInit = [this]()
		{
			auto m_FrameCommandPool = Renderer::Instance()->GetCommandPoolManager()->GetFreePool();
			m_RenderCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, 3);
			m_TransferCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, 3);
			m_ComputeCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, 3);
		};

		auto loadTextures = [this]()
		{
			const float noiseTextureWidth = 256.f;
			const float noiseTextureHeight = 256.f;

			m_ImageSelected = Image::CreateEmptyImage(noiseTextureWidth, noiseTextureHeight, vk::Format::eR8G8B8A8Snorm);
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

		};

		auto createPipelines = [this]()
		{
			auto m_DescriptorSetLayout = Renderer::Instance()->GetSetLayout(0, PipelineType::RENDER);
			auto m_DescriptorSetLayoutTex = Renderer::Instance()->GetSetLayout(1, PipelineType::RENDER);
			auto m_DescriptorSetLayoutSelected = Renderer::Instance()->GetSetLayout(0, PipelineType::COMPUTE);
			std::vector< vk::DescriptorSetLayout>layouts = { m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutSelected->GetLayout() };
			m_ComputePipeline = Pipeline::CreateComputePipeline(BASE_SPIRV_OUTPUT + "SelectedTex.spvCmp", layouts);

			auto& swapchain = Renderer::Instance()->GetSwapchain();
			auto swapchainFormat = swapchain.GetFormat();
			auto swapChainExtent = swapchain.GetExtent();
			auto swapChainDepthFormat = swapchain.GetDepthFormat();



			// render pass
			auto samples = RenderContext::GetDevice()->GetSamples();
			
			RenderPassBuilder builder;

			//Define a general attachment, with its load/store operations
			vk::AttachmentDescription msaaAttachment = AttachmentDescription(swapchainFormat, samples, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
			
			vk::AttachmentReference refMSAA = { 0,vk::ImageLayout::eColorAttachmentOptimal };
			builder.AddAttachment({ msaaAttachment ,refMSAA});

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
			builder.AddAttachment({ colorAttachmentResolve ,refResolve});




			



			


			//Renderpasses are broken down into subpasses, there's always at least one.




			builder.AddSubpass(SubpassDescription(1, &refMSAA, &refResolve, &refDepth));
			
			vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eNone, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);
			builder.AddSubpassDependency(dependency0);

			auto device = RenderContext::GetDevice();
			m_RenderPass = builder.Build(*device);




			


			// render pass



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
				auto m_DescriptorSetLayout = Renderer::Instance()->GetSetLayout(0, PipelineType::RENDER);
				auto m_DescriptorSetLayoutTex = Renderer::Instance()->GetSetLayout(1, PipelineType::RENDER);
				auto pipelineLayouts = std::vector<vk::DescriptorSetLayout>{ m_DescriptorSetLayout->GetLayout(),m_DescriptorSetLayoutTex->GetLayout() };

				specs.descriptorSetLayout = pipelineLayouts;






				m_GraphicsPipeline = Pipeline::CreateGraphicsPipeline(specs, vk::PrimitiveTopology::eTriangleList, swapchain.GetDepthFormat(), m_RenderPass, 0, true, Renderer::Instance()->GetPolygonMode());

			}

		};

		auto submitRenderCommands = [this](size_t frameIndex)
		{
			auto& renderCommandBuffer = m_RenderCommandBuffer[frameIndex];
			{
				ZoneScopedN("Sumbit render commands");
				renderCommandBuffer.BeginRendering();

				auto swapchain = Renderer::Instance()->GetSwapchain();

				vk::CommandBufferBeginInfo beginInfo = {};

				auto commandBuffer = m_RenderCommandBuffer[frameIndex].GetCommandBuffer();
				auto amount = m_IndexBuffer->GetIndexAmount();







				commandBuffer.begin(beginInfo);
				{



					TracyVkZone(ctx, commandBuffer, "Rendering ");




					m_RenderCommandBuffer[frameIndex].BeginRenderPass(&m_RenderPass, &swapchain.GetFrameBuffer(frameIndex), &swapchain.GetExtent());
					
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
					vk::DeviceSize offsets[] = { 0 };

					{
						vk::Buffer vertexBuffers[] = { m_ModelBuffer->GetBuffer() };
						commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

					}




					commandBuffer.setViewport(0, 1, &viewport);
					commandBuffer.setScissor(0, 1, &scissors);

					commandBuffer.bindIndexBuffer(m_IndexBuffer->GetBuffer(), 0, m_IndexBuffer->GetIndexType());
					commandBuffer.drawIndexed(static_cast<uint32_t>(amount), 1, 0, 0, 0);



				}
				m_RenderCommandBuffer[frameIndex].EndRenderPass();
				TracyVkCollect(ctx, commandBuffer);
				commandBuffer.end();
				renderCommandBuffer.EndRendering();

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

			UpdateTexture();
		};
		

		
	}

	void UpdateBuffer(Device& device, vk::DescriptorSet set, void* cpuBuffer, Buffer& gpuBuffer, int size )
	{
			SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(size);




			m_TransferCommandBuffer[0].BeginTransfering();
			m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_ShaderStorageBuffer.get(), (void*)m_ClickPoints.data(), size);
			m_TransferCommandBuffer[0].EndTransfering();
			m_TransferCommandBuffer[0].SubmitSingle();


			device.UpdateDescriptorSet(m_DescriptorSetSelected, 1, 1, *m_ShaderStorageBuffer, vk::DescriptorType::eStorageBuffer);
	}

	void UpdateTexture()
	{

	}
private:

	int m_BaseDesc = 0;
	int m_TexDesc = 0;
	int m_Compute = 0;
	const uint32_t MAX_POINTS = 20;
	int nextPoint = 0;
	std::vector<vk::DescriptorSet> m_DescriptorSets;
	vk::DescriptorSet m_DescriptorSetTex;
	vk::DescriptorSet m_DescriptorSetSelected;
		

	std::vector<Buffer*> m_UniformBuffers;
	std::vector<void*> uniformBuffersMapped;
	UPtr<IndexBuffer> m_IndexBuffer;
	UPtr<Buffer> m_ModelBuffer{ nullptr };
	UPtr<Buffer> m_ShaderStorageBuffer;
	SPtr<Image> m_Image;
	SPtr<Image> m_ImageSelected;
	std::vector<glm::vec2> m_ClickPoints;


	std::vector<CommandBuffer> m_RenderCommandBuffer,
		m_TransferCommandBuffer, m_ComputeCommandBuffer;


	

	UPtr<Pipeline> m_ComputePipeline;
	UPtr<Pipeline> m_GraphicsPipeline;
	vk::RenderPass m_RenderPass;
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
