#pragma once
#include "vulkan/vulkan.hpp"
#include <string>
#include "RenderContext.h"
#include "Device.h"
#include "../Types.h"
#include "../Memory.h"
#include <vector>
namespace Voidstar
{
	struct VOIDSTAR_API  ImageSpecs
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

	inline int FormatToSize(vk::Format format)
	{
		switch (format)
		{
			// 8-bit formats
		case vk::Format::eR8Unorm:
		case vk::Format::eR8Snorm:
		case vk::Format::eR8Uscaled:
		case vk::Format::eR8Sscaled:
		case vk::Format::eR8Uint:
		case vk::Format::eR8Sint:
			return 1;

			// 16-bit formats
		case vk::Format::eR16Unorm:
		case vk::Format::eR16Snorm:
		case vk::Format::eR16Uscaled:
		case vk::Format::eR16Sscaled:
		case vk::Format::eR16Uint:
		case vk::Format::eR16Sint:
		case vk::Format::eR16Sfloat:
		case vk::Format::eD16Unorm:
		case vk::Format::eR8G8Unorm:
		case vk::Format::eR8G8Snorm:
		case vk::Format::eR8G8Uscaled:
		case vk::Format::eR8G8Sscaled:
		case vk::Format::eR8G8Uint:
		case vk::Format::eR8G8Sint:
			return 2;

			// 32-bit formats
		case vk::Format::eR32Uint:
		case vk::Format::eR32Sint:
		case vk::Format::eR32Sfloat:
		case vk::Format::eD32Sfloat:
		case vk::Format::eS8Uint:
		case vk::Format::eR16G16Unorm:
		case vk::Format::eR16G16Snorm:
		case vk::Format::eR16G16Uscaled:
		case vk::Format::eR16G16Sscaled:
		case vk::Format::eR16G16Uint:
		case vk::Format::eR16G16Sint:
		case vk::Format::eR16G16Sfloat:
			return 4;

			// 64-bit formats
		case vk::Format::eR64Uint:
		case vk::Format::eR64Sint:
		case vk::Format::eR64Sfloat:
		case vk::Format::eR32G32Uint:
		case vk::Format::eR32G32Sint:
		case vk::Format::eR32G32Sfloat:
		case vk::Format::eR16G16B16A16Unorm:
		case vk::Format::eR16G16B16A16Snorm:
		case vk::Format::eR16G16B16A16Uscaled:
		case vk::Format::eR16G16B16A16Sscaled:
		case vk::Format::eR16G16B16A16Uint:
		case vk::Format::eR16G16B16A16Sint:
		case vk::Format::eR16G16B16A16Sfloat:
			return 8;

			// 96-bit formats
		case vk::Format::eR32G32B32Uint:
		case vk::Format::eR32G32B32Sint:
		case vk::Format::eR32G32B32Sfloat:
			return 12;

			// 128-bit formats
		case vk::Format::eR32G32B32A32Uint:
		case vk::Format::eR32G32B32A32Sint:
		case vk::Format::eR32G32B32A32Sfloat:
			return 16;

			// Depth/stencil formats
		case vk::Format::eD16UnormS8Uint:
		case vk::Format::eD24UnormS8Uint:
		case vk::Format::eD32SfloatS8Uint:
			return 4;


		default:
			return -1; // Unsupported format
		}
	};
	inline size_t GetImageSize(int width, int height, vk::Format format)
	{
		assert(width != -1);
		assert(height!= -1);
		return width * height * FormatToSize(format);
	}
	class VOIDSTAR_API Image
	{
	public:
		Image() = default;
		Image(const Image& image) = delete;
		

		// create image
		static SPtr<Image> CreateImageFrom(Memory& mem, int w, int h);
		static VkImageView CreateImageView(vk::Image& image, vk::Format format, vk::ImageAspectFlags aspect, vk::ImageViewType viewType =  vk::ImageViewType::e2D, int mipmap = 1, int layers = 1);
		static vk::Sampler CreateSampler(vk::Filter min, vk::Filter mag);
		static vk::Image CreateVKImage(ImageSpecs& specs, vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1, int mipmap = 1);
		static vk::DeviceMemory CreateMemory(vk::Image& image, ImageSpecs& specs);
		static SPtr<Image> CreateImage(std::string_view path);
		
		static SPtr<Image> CreateCubemap(std::vector<std::string> pathes);
		void Fill(glm::vec4  value, CommandBuffer& cmd, SPtr<Buffer> stageBuffer, int bufferOffset);
		static SPtr<Image> CreateEmptyImage( int width, int height,vk::Format format,
			vk::ImageUsageFlags usage,
			int mipLevels = 1,
			vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1,
			vk::Filter minFilter = vk::Filter::eNearest, vk::Filter magFilter = vk::Filter::eNearest,
			int layers = 1,
			vk::ImageViewType viewType= vk::ImageViewType::e2D);
		static SPtr<Image> CreateEmpty3DImage(int width, int height, int depth, vk::Format format);
		static void UpdateRegionWithImage(Memory& mem, size_t width, size_t height, SPtr<Image> parentImage, vk::Offset3D offset, int layer);
		~Image();

		
		vk::Sampler  GetSampler() { return m_Sampler; }
		vk::ImageLayout GetLayout() { return m_ImageLayout; }
		vk::CommandPool GetCommandPool() { return m_CommandPool; }
		void SetFormat(vk::Format format)
		{
			m_Format = format;
		}
		void SetLayout(vk::ImageLayout layout)
		{
			m_ImageLayout= layout;
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
		vk::Image GetImage() {
			return m_Image;
		}
		vk::Format GetFormat()
		{
			return m_Format;
		}
		vk::ImageView GetImageView() { return m_ImageView; }
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
		vk::DeviceMemory GetMemory()
		{
			assert(!m_ImageMemory);
			return m_ImageMemory;
		}
		static vk::Format GetFormat(vk::PhysicalDevice physicalDevice,
			const std::vector<vk::Format>& candidates,
			vk::ImageTiling tiling, vk::FormatFeatureFlags features);
		void GenerateMipmaps(uint32_t mipLevels);
		std::vector<SPtr<Image>> GenerateEmptyMipmapsAsImages(uint32_t mipLevels);
		int GetSize()
		{
			assert(m_Size != -1);
			return m_Size;
		}

		static Memory LoadImageCPU(const std::string& path,
			size_t& width_,
			size_t& height_);
	private:
		static void InitVulkanImageFromRGBA8(
			Image& image,
			const void* rgbaPixels,
			int width,
			int height,
			bool generateMips,
			vk::Filter samplerFilter = vk::Filter::eLinear,
			vk::SamplerAddressMode addressMode = vk::SamplerAddressMode::eRepeat);
		void GenerateMipmaps(VkImage image,VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void* LoadImageRaw();
	private:
		friend class Renderer;
		friend class Swapchain;
		friend class CommandBuffer;
		friend class Device;
		friend class AttachmentManager;
		int m_Size = -1;
		int m_Depth = 0, m_Channels;
		int m_MipMapLevels = 1;
		int m_Width, m_Height;
		int layers = 1;
		vk::Image m_Image = VK_NULL_HANDLE;
		vk::ImageView m_ImageView = VK_NULL_HANDLE;
		vk::Format m_Format;
		vk::SampleCountFlagBits m_Sample;
		
		vk::Sampler m_Sampler = VK_NULL_HANDLE;
		vk::DeviceMemory m_ImageMemory = VK_NULL_HANDLE;
		vk::CommandPool m_CommandPool;
		
		vk::ImageLayout m_ImageLayout = vk::ImageLayout::eUndefined;
	};
}