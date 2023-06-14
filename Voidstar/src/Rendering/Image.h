#pragma once
#include "vulkan/vulkan.hpp"
#include <string>
#include "../Types.h"
namespace Voidstar
{
	class Image
	{
	public:
		// create image
		static VkImageView CreateImageView(vk::Image& image, vk::Format format);
		static SPtr<Image> CreateImage(std::string path, vk::DescriptorSet descriptorSet);
		~Image();
		void Bind();
	private:
		void* LoadImageRaw();
	private:
		int m_Width, m_Height, m_Channels;
		vk::Image m_Image;
		vk::DeviceMemory m_ImageMemory;
		vk::ImageView m_ImageView;
		vk::Sampler m_Sampler;
	};
}