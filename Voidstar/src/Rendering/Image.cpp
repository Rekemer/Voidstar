#include"Prereq.h"
#define STB_IMAGE_IMPLEMENTATION
#include "Image.h"
#include "RenderContext.h"
#include "Device.h"
#include "../../vendor/stb_image.h"
#include "Log.h"
#include "Buffer.h"
#include "Queue.h"
namespace Voidstar
{
	VkImageView Image::CreateImageView(vk::Image& image, vk::Format format, vk::ImageAspectFlags aspect)
	{
		vk::ImageViewCreateInfo createInfo = {};
		createInfo.image = image;
		createInfo.viewType = vk::ImageViewType::e2D;
		createInfo.format = format;
		createInfo.components.r = vk::ComponentSwizzle::eIdentity;
		createInfo.components.g = vk::ComponentSwizzle::eIdentity;
		createInfo.components.b = vk::ComponentSwizzle::eIdentity;
		createInfo.components.a = vk::ComponentSwizzle::eIdentity;
		createInfo.subresourceRange.aspectMask = aspect;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		auto device = RenderContext::GetDevice();
		return device->GetDevice().createImageView(createInfo);
	}
	vk::Image Image::CreateVKImage(ImageSpecs& specs)
	{
		auto device = RenderContext::GetDevice();
		vk::ImageCreateInfo imageInfo;
		imageInfo.flags = vk::ImageCreateFlagBits();
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent = vk::Extent3D(specs.width, specs.height, 1);
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = specs.format;
		imageInfo.tiling = specs.tiling;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = specs.usage;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		return device->GetDevice().createImage(imageInfo);
	}
	vk::DeviceMemory Image::CreateMemory(vk::Image& image, ImageSpecs& specs)
	{
		auto device = RenderContext::GetDevice();
		vk::MemoryRequirements requirements = device->GetDevice().getImageMemoryRequirements(image);

		vk::MemoryAllocateInfo allocation;
		allocation.allocationSize = requirements.size;
		allocation.memoryTypeIndex = device->FindMemoryTypeIndex(
		 requirements.memoryTypeBits, specs.memoryProperties
		);

		vk::DeviceMemory imageMemory = device->GetDevice().allocateMemory(allocation);
		device->GetDevice().bindImageMemory(image, imageMemory, 0);
		return imageMemory;
		
	}
	SPtr<Image> Image::CreateImage(std::string path,vk::DescriptorSet descriptorSet)
	{

		auto image = CreateUPtr<Image>();
		stbi_set_flip_vertically_on_load(true);
		auto pixels = stbi_load(path.c_str(), &image->m_Width, &image->m_Height, &image->m_Channels, STBI_rgb_alpha);
		if (!pixels) {
			Log::GetLog()->error("Unable to load: {0}",path);
		}


		//vk::ImageCreateInfo imageInfo;
		//imageInfo.flags = vk::ImageCreateFlagBits();
		//imageInfo.imageType = vk::ImageType::e2D;
		//imageInfo.extent = vk::Extent3D(image->m_Width, image->m_Height, 1);
		//imageInfo.mipLevels = 1;
		//imageInfo.arrayLayers = 1;
		//imageInfo.format = vk::Format::eR8G8B8A8Unorm;
		//imageInfo.tiling = vk::ImageTiling::eOptimal;
		//imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		//imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst | //vk::ImageUsageFlagBits::eSampled;
		//imageInfo.sharingMode = vk::SharingMode::eExclusive;
		//imageInfo.samples = vk::SampleCountFlagBits::e1;
		//memory prop = vk::MemoryPropertyFlagBits::eDeviceLocal

		auto device = RenderContext::GetDevice();
		ImageSpecs specs;
		specs.width = image->m_Width;
		specs.height = image->m_Height;
		specs.format = vk::Format::eR8G8B8A8Unorm;
		specs.tiling = vk::ImageTiling::eOptimal;
		specs.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		specs.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		image->m_Format = specs.format;
		try {
			image->m_Image = CreateVKImage(specs);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Unable to make image: {0}", path);
		}

		//vk::MemoryRequirements requirements = device->GetDevice().getImageMemoryRequirements(image->m_Image);
		//
		//vk::MemoryAllocateInfo allocation;
		//allocation.allocationSize = requirements.size;
		//
		//
		//allocation.memoryTypeIndex = device->FindMemoryTypeIndex(requirements.memoryTypeBits, //vk::MemoryPropertyFlagBits::eDeviceLocal
		//);
		try 
		{
			image->m_ImageMemory = CreateMemory(image->m_Image,specs);
		}
		catch (vk::SystemError err) 
		{
			Log::GetLog()->error("Unable to allocate memory for image: {0}", path);
		}

		// populate memory with data
		auto imageSize = image->m_Width * image->m_Height * 4;
		auto buffer = Buffer::CreateStagingBuffer(imageSize);



		
		//graphicsQueue->Transfer(stagingBuffer.get(), m_Buffer, (void*)vertices.data(), sizeof(vertices));





		//...then fill it,
		void* writeLocation = device->GetDevice().mapMemory(buffer->GetMemory(), 0, imageSize);
		memcpy(writeLocation, pixels, imageSize);
		device->GetDevice().unmapMemory(buffer->GetMemory());

		//then transfer it to image memory
		auto graphicsQueue = RenderContext::GetGraphicsQueue();
		graphicsQueue->BeginTransfering();
		graphicsQueue->ChangeImageLayout(&image->m_Image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		graphicsQueue->EndTransfering();

		graphicsQueue->BeginTransfering();
		graphicsQueue->CopyBufferToImage(buffer.get(), &image->m_Image, image->m_Width, image->m_Height);
		graphicsQueue->EndTransfering();
		

		
		graphicsQueue->BeginTransfering();
		graphicsQueue->ChangeImageLayout(&image->m_Image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
		graphicsQueue->EndTransfering();


		free(pixels);


		image->m_ImageView = CreateImageView(image->m_Image, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor);


		/*
	typedef struct VkSamplerCreateInfo {
		VkStructureType         sType;
		const void* pNext;
		VkSamplerCreateFlags    flags;
		VkFilter                magFilter;
		VkFilter                minFilter;
		VkSamplerMipmapMode     mipmapMode;
		VkSamplerAddressMode    addressModeU;
		VkSamplerAddressMode    addressModeV;
		VkSamplerAddressMode    addressModeW;
		float                   mipLodBias;
		VkBool32                anisotropyEnable;
		float                   maxAnisotropy;
		VkBool32                compareEnable;
		VkCompareOp             compareOp;
		float                   minLod;
		float                   maxLod;
		VkBorderColor           borderColor;
		VkBool32                unnormalizedCoordinates;
	} VkSamplerCreateInfo;
	*/
		vk::SamplerCreateInfo samplerInfo;
		samplerInfo.flags = vk::SamplerCreateFlags();
		samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.magFilter = vk::Filter::eLinear;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;

		samplerInfo.anisotropyEnable = false;
		samplerInfo.maxAnisotropy = 1.0f;

		samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		samplerInfo.unnormalizedCoordinates = false;
		samplerInfo.compareEnable = false;
		samplerInfo.compareOp = vk::CompareOp::eAlways;

		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		try 
		{
			image->m_Sampler= device->GetDevice().createSampler(samplerInfo);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to make sampler for image: {0}", path);

		}


		vk::DescriptorImageInfo imageDescriptor;
		imageDescriptor.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imageDescriptor.imageView = image->m_ImageView;
		imageDescriptor.sampler = image->m_Sampler;

		vk::WriteDescriptorSet descriptorWrite;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageDescriptor;

		device->GetDevice().updateDescriptorSets(descriptorWrite, nullptr);

		return image;
	}
	Image::~Image()
	{
		auto logicalDevice = RenderContext::GetDevice()->GetDevice();
		logicalDevice.freeMemory(m_ImageMemory);
		logicalDevice.destroyImage(m_Image);
		logicalDevice.destroyImageView(m_ImageView);
		logicalDevice.destroySampler(m_Sampler);
	}
	vk::Format Image::GetFormat(vk::PhysicalDevice physicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
	{
		for (vk::Format format : candidates) {

			vk::FormatProperties properties = physicalDevice.getFormatProperties(format);

			/*
			typedef struct VkFormatProperties {
				VkFormatFeatureFlags    linearTilingFeatures;
				VkFormatFeatureFlags    optimalTilingFeatures;
				VkFormatFeatureFlags    bufferFeatures;
			} VkFormatProperties;
			*/

			if (tiling == vk::ImageTiling::eLinear
				&& (properties.linearTilingFeatures & features) == features) {
				return format;
			}

			if (tiling == vk::ImageTiling::eOptimal
				&& (properties.optimalTilingFeatures & features) == features) {
				return format;
			}

			std::runtime_error("Unable to find suitable format");
		}
	}
}