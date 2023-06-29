#pragma once
#include "vulkan/vulkan.hpp"
#include "../Prereq.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include"Queue.h"
#include"../Types.h"
namespace Voidstar
{
	class Window;
	class Surface;
	class SwapChainSupportDetails;
	class Buffer;
	class Camera;
	class Application;
	class Image;
	class DescriptorPool;
	class Model;




	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct Particle {
		glm::vec2 position;
		glm::vec2 velocity;
		glm::vec4 color;

		static vk::VertexInputBindingDescription GetBindingDescription() {
			vk::VertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Particle);
			bindingDescription.inputRate = vk::VertexInputRate::eVertex;
			return bindingDescription;
		}

		static std::array<vk::VertexInputAttributeDescription, 2> GetAttributeDescriptions() {
			std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
			attributeDescriptions[0].offset = offsetof(Particle, position);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = vk::Format::eR32G32B32A32Sfloat;
			attributeDescriptions[1].offset = offsetof(Particle, color);

			return attributeDescriptions;
		}

	};


	struct GraphicsPipelineSpecification
	{
		
		vk::Device device;
		std::string vertexFilepath;
		std::string fragmentFilepath;
		vk::SampleCountFlagBits samples;
		vk::Extent2D swapchainExtent;
		vk::Format swapchainImageFormat;
		vk::VertexInputBindingDescription bindingDescription;
		std::array<vk::VertexInputAttributeDescription, 2>  attributeDescription;
		std::vector<vk::DescriptorSetLayout> descriptorSetLayout;
	};

	struct GraphicsPipeline
	{
		vk::PipelineLayout layout;
		vk::RenderPass renderpass;
		vk::Pipeline pipeline;
	};
	class Instance;
	class Device;
	class Swapchain;
	class DescriptorSetLayout;
	class Renderer
	{
	public:
		Renderer(size_t screenWidth, size_t screenHeight, std::shared_ptr<Window> window, Application* app );
		void Render();
		~Renderer();
	private:
		void CreateInstance();
		void CreateDebugMessenger();
		void CreateSurface();
		void CreateDevice();
		void CreatePipeline();
		void CreateFramebuffers();
		GraphicsPipeline CreatePipeline(GraphicsPipelineSpecification& spec);
		void DestroySwapchain();
		void CreateSyncObjects();
		void CreateMSAAFrame();
		
		void RecordCommandBuffer(uint32_t imageIndex);
		void UpdateUniformBuffer(uint32_t imageIndex);
		void RecreateSwapchain();
	
	private:
		int m_ViewportWidth, m_ViewportHeight;
		Instance* m_Instance;
		Device* m_Device;
		UPtr<Swapchain> m_Swapchain;
		Buffer* m_Buffer{nullptr};
		IndexBuffer* m_IndexBuffer;
		Application* m_App;
		
		SPtr<Image> m_Image;
		SPtr<Model> m_Model;


		vk::Image m_MsaaImage;
		vk::DeviceMemory m_MsaaImageMemory;
		vk::ImageView m_MsaaImageView;


		SPtr<DescriptorPool> m_DescriptorPool;

		// can be in one buffer?
		std::vector<Buffer*> m_UniformBuffers;
		std::vector<void*> uniformBuffersMapped;
		
		DescriptorSetLayout* m_DescriptorSetLayout;
		DescriptorSetLayout* m_DescriptorSetLayoutTex;
		std::vector<vk::DescriptorSetLayout> m_DescriptorSetLayouts;
		std::vector<vk::DescriptorSet> m_DescriptorSets;
		std::vector<UPtr<Buffer>> m_ShaderStorageBuffers;
		vk::CommandPool m_CommandComputePool;
		std::vector<vk::CommandBuffer> m_CommandComputeBuffers;

		SPtr<DescriptorPool> m_DescriptorPoolTex;
		vk::DescriptorSet m_DescriptorSetTex;
		SPtr<DescriptorPool> m_ComputePool;
		DescriptorSetLayout* m_ComputeSetLayout;
		std::vector<vk::DescriptorSet> m_ComputeDescriptorSets;
		//debug callback
		vk::DebugUtilsMessengerEXT m_DebugMessenger;
		//dynamic instance dispatcher
		vk::DispatchLoaderDynamic m_Dldi;



		vk::SurfaceKHR m_Surface;
		
	

		

		vk::RenderPass m_RenderPass;
		vk::PipelineLayout m_PipelineLayout;
		vk::PipelineLayout m_ComputePipelineLayout;
		vk::Pipeline m_Pipeline;
		vk::Pipeline m_ComputePipeline;
		UPtr<Queue> m_GraphicsQueue;


		vk::Semaphore m_ImageAvailableSemaphore;
		vk::Semaphore m_RenderFinishedSemaphore;
		vk::Fence	m_InFlightFence;

		std::vector<vk::Semaphore> m_ComputeFinishedSemaphores;
		std::vector<vk::Fence> m_ComputeInFlightFences;

		SPtr<Window> m_Window;


		
	};

}

