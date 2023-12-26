#pragma once
#include "vulkan/vulkan.hpp"
#include <string>
#include "RenderContext.h"
#include "Device.h"
#include "../Types.h"
namespace Voidstar
{
	struct ImageSpecs
	{
		int width, height, depth = 1;
		
		vk::ImageTiling tiling;
		vk::ImageUsageFlags usage;
		vk::MemoryPropertyFlags memoryProperties;
		vk::Format format;
		vk::Filter minFilter;
		vk::Filter magFilter;
		vk::SampleCountFlagBits samples;
		vk::ImageAspectFlagBits imageAspect = vk::ImageAspectFlagBits::eColor;
		int arrayCount = 1;
		vk::ImageCreateFlags flags;
		vk::ImageType imageType = vk::ImageType::e2D;
	};


	class SwapchainImage
	{
	public:
		friend class Swapchain;
		vk::Image GetImage() {
			return m_Image;
		}
		vk::Format GetFormat()
		{
			return m_Format;
		}
		vk::ImageView GetImageView() { return m_ImageView; }
		void Destroy()
		{
			auto logicalDevice = RenderContext::GetDevice()->GetDevice();
			logicalDevice.destroyImage(m_Image);
			logicalDevice.destroyImageView(m_ImageView);
		}
		int GetWidth()
		{
			return m_Width;
		}
		int GetHeight()
		{
			return m_Height;
		}
		void SetSample(vk::SampleCountFlagBits sample)
		{
			m_Sample = sample;
		}
		void SetHeight(int height)
		{
			m_Height = height;
		}
		void SetWidth(int height)
		{
			m_Width = height;
		}
		vk::SampleCountFlagBits GetSample()
		{
			return m_Sample;
		}

	protected:
		int m_Width, m_Height;
		vk::Image m_Image;
		vk::ImageView m_ImageView;
		vk::Format m_Format;
		vk::SampleCountFlagBits m_Sample;
	};
	class Image final : public SwapchainImage
	{
	public:
		Image() = default;
		Image(const Image& image) = delete;
		//Image(Image&& image);
		// create image
		static VkImageView CreateImageView(vk::Image& image, vk::Format format, vk::ImageAspectFlags aspect, vk::ImageViewType viewType =  vk::ImageViewType::e2D, int mipmap = 1, int layers = 1);

		static vk::Image CreateVKImage(ImageSpecs& specs, vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1, int mipmap = 1);
		static vk::DeviceMemory CreateMemory(vk::Image& image, ImageSpecs& specs);
		static SPtr<Image> CreateImage(std::string path);
		static SPtr<Image> CreateCubemap(std::vector<std::string> pathes);

		static SPtr<Image> CreateEmptyImage( int width, int height,vk::Format format,
			vk::ImageUsageFlags usage,
			vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1,
			vk::Filter minFilter = vk::Filter::eNearest, vk::Filter magFilter = vk::Filter::eNearest);
		static SPtr<Image> CreateEmpty3DImage(int width, int height, int depth, vk::Format format);
		~Image();
		void Bind();
		
		

		
		vk::Sampler  GetSampler() { return m_Sampler; }
		vk::ImageLayout GetLayout() { return m_ImageLayout; }
		void SetFormat(vk::Format format)
		{
			m_Format = format;
		}
		void SetView(vk::ImageView view)
		{
			m_ImageView= view;
		}
		void SetImage(vk::Image image)
		{
			m_Image= image;
		}
		void SetMemory(vk::DeviceMemory memory)
		{
			m_ImageMemory = memory;
		}
		
		
		static vk::Format GetFormat(vk::PhysicalDevice physicalDevice,
			const std::vector<vk::Format>& candidates,
			vk::ImageTiling tiling, vk::FormatFeatureFlags features);
	private:
		void GenerateMipmaps(VkImage image,VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void* LoadImageRaw();
	private:
		friend class Renderer;
		friend class CommandBuffer;
		friend class Device;
		int m_Depth = 0, m_Channels;
		int m_MipMapLevels;
		
		
		vk::Sampler m_Sampler;
		vk::DeviceMemory m_ImageMemory;
		vk::CommandPool m_CommandPool;
		
		vk::ImageLayout m_ImageLayout = vk::ImageLayout::eUndefined;
	};
}