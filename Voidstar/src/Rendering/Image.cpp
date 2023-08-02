#include"Prereq.h"
#define STB_IMAGE_IMPLEMENTATION
#include "Image.h"
#include "RenderContext.h"
#include "Device.h"
#include "stb_image.h"
#include "Log.h"
#include "Buffer.h"
#include "CommandBuffer.h"
#include "CommandPoolManager.h"
#include "Renderer.h"
namespace Voidstar
{
	VkImageView Image::CreateImageView(vk::Image& image, vk::Format format, vk::ImageAspectFlags aspect, int mipmap)
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
		createInfo.subresourceRange.levelCount = mipmap;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		auto device = RenderContext::GetDevice();
		return device->GetDevice().createImageView(createInfo);
	}
	vk::Image Image::CreateVKImage(ImageSpecs& specs, vk::SampleCountFlagBits samples, int mipmap )
	{
		auto device = RenderContext::GetDevice();
		vk::ImageCreateInfo imageInfo;
		imageInfo.flags = vk::ImageCreateFlagBits();
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent = vk::Extent3D(specs.width, specs.height, 1);
		imageInfo.mipLevels = mipmap;
		imageInfo.arrayLayers = 1;
		imageInfo.format = specs.format;
		imageInfo.tiling = specs.tiling;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = specs.usage;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.samples = samples;
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
		
		image->m_CommandPool = Renderer::Instance()->GetCommandPoolManager()->GetFreePool();
		stbi_set_flip_vertically_on_load(true);
		auto pixels = stbi_load(path.c_str(), &image->m_Width, &image->m_Height, &image->m_Channels, STBI_rgb_alpha);
		if (!pixels) {
			Log::GetLog()->error("Unable to load: {0}", path);
		}

		auto mipMaps = static_cast<uint32_t>(std::floor(std::log2(std::max(image->m_Width, image->m_Height)))) + 1;
	

		auto device = RenderContext::GetDevice();
		ImageSpecs specs;
		specs.width = image->m_Width;
		specs.height = image->m_Height;
		specs.format = vk::Format::eR8G8B8A8Unorm;
		specs.tiling = vk::ImageTiling::eOptimal;
		specs.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
		specs.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		image->m_Format = specs.format;
		try {
			image->m_Image = CreateVKImage(specs,vk::SampleCountFlagBits::e1, mipMaps);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Unable to make image: {0}", path);
		}

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







		//...then fill it,
		void* writeLocation = device->GetDevice().mapMemory(buffer->GetMemory(), 0, imageSize);
		memcpy(writeLocation, pixels, imageSize);
		device->GetDevice().unmapMemory(buffer->GetMemory());

		//then transfer it to image memory
		auto commandBuffer = CommandBuffer::CreateBuffer(image->m_CommandPool,vk::CommandBufferLevel::ePrimary);


		commandBuffer.BeginTransfering();
		commandBuffer.ChangeImageLayout(image.get(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipMaps);
		commandBuffer.EndTransfering();
		commandBuffer.SubmitSingle();

		commandBuffer.BeginTransfering();
		commandBuffer.CopyBufferToImage(buffer.get(), &image->m_Image, image->m_Width, image->m_Height);
		commandBuffer.EndTransfering();
		commandBuffer.SubmitSingle();
		
		
		
		//commandBuffer.BeginTransfering();
		//commandBuffer.ChangeImageLayout(image.get(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, mipMaps);
		//commandBuffer.EndTransfering();
		//commandBuffer.SubmitSingle();

		image->GenerateMipmaps(image->m_Image, (VkFormat)image->m_Format, image->m_Width, image->m_Height,mipMaps);


		commandBuffer.Free();
		free(pixels);


		image->m_ImageView = CreateImageView(image->m_Image, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, mipMaps);


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
		samplerInfo.maxLod = (float)mipMaps;
		//samplerInfo.maxLod = 0;

		try 
		{
			image->m_Sampler= device->GetDevice().createSampler(samplerInfo);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to make sampler for image: {0}", path);

		}




		return image;
	}
	SPtr<Image> Image::CreateEmptyImage(int width, int height)
	{
		auto image = CreateUPtr<Image>();

		image->m_CommandPool = Renderer::Instance()->GetCommandPoolManager()->GetFreePool();
		image->m_Width = width;
		image->m_Height = height;


		auto device = RenderContext::GetDevice();
		ImageSpecs specs;
		specs.width = image->m_Width;
		specs.height = image->m_Height;
		specs.format = vk::Format::eR8G8B8A8Unorm;
		specs.tiling = vk::ImageTiling::eOptimal;
		specs.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
		specs.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		image->m_Format = specs.format;
		try {
			image->m_Image = CreateVKImage(specs, vk::SampleCountFlagBits::e1);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Unable to make empty image: ");
		}

		try
		{
			image->m_ImageMemory = CreateMemory(image->m_Image, specs);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Unable to allocate memory for empty image");
		}

		// populate memory with data
		auto imageSize = image->m_Width * image->m_Height * 4;
		auto buffer = Buffer::CreateStagingBuffer(imageSize);













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
			image->m_Sampler = device->GetDevice().createSampler(samplerInfo);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to make sampler for empty image");

		}

		//then transfer it to image memory
		auto commandBuffer = CommandBuffer::CreateBuffer(image->m_CommandPool, vk::CommandBufferLevel::ePrimary);


		commandBuffer.BeginTransfering();
		commandBuffer.ChangeImageLayout(image.get(), vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
		commandBuffer.EndTransfering();
		commandBuffer.SubmitSingle();

		commandBuffer.Free();


		

		return image;
	}
	Image::~Image()
	{
		auto logicalDevice = RenderContext::GetDevice()->GetDevice();
		logicalDevice.freeMemory(m_ImageMemory);
		logicalDevice.destroyImage(m_Image);
		logicalDevice.destroyImageView(m_ImageView);
		logicalDevice.destroySampler(m_Sampler);
		Renderer::Instance()->GetCommandPoolManager()->FreePool(m_CommandPool);
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
	void Image::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
	{
		
		// Check if image format supports linear blitting

		auto physicalDevice = RenderContext::GetDevice()->GetDevicePhys();
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
			throw std::runtime_error("texture image format does not support linear blitting!");
		}
		assert(m_CommandPool);
		auto commandBuffer = CommandBuffer::CreateBuffer(m_CommandPool,vk::CommandBufferLevel::ePrimary);
		commandBuffer.BeginTransfering();
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;


		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer.GetCommandBuffer(),
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);


			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;


			vkCmdBlitImage(commandBuffer.GetCommandBuffer(),
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer.GetCommandBuffer(),
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);
			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;

			
		}
		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer.GetCommandBuffer(),
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);
		commandBuffer.EndTransfering();
		commandBuffer.SubmitSingle();
		commandBuffer.Free();
	}
}