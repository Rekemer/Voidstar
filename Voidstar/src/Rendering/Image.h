#pragma once
#include "vulkan/vulkan.hpp"
#include <string>
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

		int arrayCount = 1;
		vk::ImageCreateFlags flags;
		vk::ImageType imageType = vk::ImageType::e2D;
	};

	class Image
	{
	public:
		Image() = default;
		Image(const Image& image) = delete;
		// create image
		static VkImageView CreateImageView(vk::Image& image, vk::Format format, vk::ImageAspectFlags aspect, vk::ImageViewType viewType =  vk::ImageViewType::e2D, int mipmap = 1, int layers = 1);

		static vk::Image CreateVKImage(ImageSpecs& specs, vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1, int mipmap = 1);
		static vk::DeviceMemory CreateMemory(vk::Image& image, ImageSpecs& specs);
		static SPtr<Image> CreateImage(std::string path);
		static SPtr<Image> CreateCubemap(std::vector<std::string> pathes);

		static SPtr<Image> CreateEmptyImage( int width, int height,vk::Format format);
		static SPtr<Image> CreateEmpty3DImage(int width, int height, int depth, vk::Format format);
		~Image();
		void Bind();
		static vk::Format GetFormat(vk::PhysicalDevice physicalDevice,
			const std::vector<vk::Format>& candidates,
			vk::ImageTiling tiling, vk::FormatFeatureFlags features);
		vk::Format GetFormat()
		{
			return m_Format;
		}

		vk::ImageView GetImageView() { return m_ImageView; }
		vk::Sampler  GetSampler() { return m_Sampler; }
		vk::ImageLayout GetLayout() { return m_ImageLayout; }
		void SetFormat(vk::Format format)
		{
			m_Format = format;
		}
	private:
		void GenerateMipmaps(VkImage image,VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void* LoadImageRaw();
	private:
		friend class Renderer;
		friend class CommandBuffer;
		friend class Device;
		int m_Width, m_Height,m_Depth = 0, m_Channels;
		int m_MipMapLevels;
		vk::Image m_Image;
		vk::Format m_Format;
		vk::DeviceMemory m_ImageMemory;
		vk::ImageView m_ImageView;
		vk::Sampler m_Sampler;
		vk::CommandPool m_CommandPool;
		vk::ImageLayout m_ImageLayout = vk::ImageLayout::eUndefined;
	};
}