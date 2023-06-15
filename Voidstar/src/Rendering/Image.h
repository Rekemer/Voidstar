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
		static VkImageView CreateImageView(vk::Image& image, vk::Format format, vk::ImageAspectFlags aspect);

		static vk::Image CreateVKImage(ImageSpecs& specs);
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
		void* LoadImageRaw();
	private:
		int m_Width, m_Height, m_Channels;
		vk::Image m_Image;
		vk::Format m_Format;
		vk::DeviceMemory m_ImageMemory;
		vk::ImageView m_ImageView;
		vk::Sampler m_Sampler;
	};
}