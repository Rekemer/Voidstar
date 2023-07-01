#pragma once
#include "vulkan/vulkan.hpp"
#include <string>
#include "../Types.h"
namespace Voidstar
{

	struct ImageSpecs
	{
		int width, height;
		
		vk::ImageTiling tiling;
		vk::ImageUsageFlags usage;
		vk::MemoryPropertyFlags memoryProperties;
		vk::Format format;
	};

	class Image
	{
	public:
		// create image
		static VkImageView CreateImageView(vk::Image& image, vk::Format format, vk::ImageAspectFlags aspect, int mipmap = 1);

		static vk::Image CreateVKImage(ImageSpecs& specs, vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1, int mipmap = 1);
		static vk::DeviceMemory CreateMemory(vk::Image& image, ImageSpecs& specs);
		static SPtr<Image> CreateImage(std::string path, vk::DescriptorSet descriptorSet);
		~Image();
		void Bind();
		static vk::Format GetFormat(vk::PhysicalDevice physicalDevice,
			const std::vector<vk::Format>& candidates,
			vk::ImageTiling tiling, vk::FormatFeatureFlags features);
		vk::Format GetFormat()
		{
			return m_Format;
		}
		void SetFormat(vk::Format format)
		{
			m_Format = format;
		}
	private:
		void GenerateMipmaps(VkImage image,VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void* LoadImageRaw();
	private:
		int m_Width, m_Height, m_Channels;
		int m_MipMapLevels;
		vk::Image m_Image;
		vk::Format m_Format;
		vk::DeviceMemory m_ImageMemory;
		vk::ImageView m_ImageView;
		vk::Sampler m_Sampler;
		vk::CommandPool m_CommandPool;
	};
}