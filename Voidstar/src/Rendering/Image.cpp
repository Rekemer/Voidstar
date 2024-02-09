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
#include"tracy/Tracy.hpp"
#include <mutex>
namespace Voidstar
{
	int FormatToSize(vk::Format format)
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
	}

	//Image::Image(Image&& image)
	//{
	//	m_Image = image.m_Image ;
	//	m_ImageView = image.m_ImageView ;
	//	m_Sampler = image.m_Sampler ;
	//	m_ImageMemory = image.m_ImageMemory ;
	//	m_CommandPool = image.m_CommandPool;
	//
	//	image.m_Image = VK_NULL_HANDLE;
	//	image.m_ImageView = VK_NULL_HANDLE;
	//	image.m_Sampler = VK_NULL_HANDLE;
	//	image.m_ImageMemory = VK_NULL_HANDLE;
	//	image.m_CommandPool = VK_NULL_HANDLE;
	//
	//}
	VkImageView Image::CreateImageView(vk::Image& image, vk::Format format, vk::ImageAspectFlags aspect, vk::ImageViewType viewType,int mipmap, int layers)
	{
		vk::ImageViewCreateInfo createInfo = {};
		createInfo.image = image;
		createInfo.viewType = viewType;
		createInfo.format = format;
		createInfo.components.r = vk::ComponentSwizzle::eIdentity;
		createInfo.components.g = vk::ComponentSwizzle::eIdentity;
		createInfo.components.b = vk::ComponentSwizzle::eIdentity;
		createInfo.components.a = vk::ComponentSwizzle::eIdentity;
		createInfo.subresourceRange.aspectMask = aspect;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = mipmap;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = layers;

		auto device = RenderContext::GetDevice();
		return device->GetDevice().createImageView(createInfo);
	}
	

	vk::Sampler Image::CreateSampler(vk::Filter min, vk::Filter mag)
	{
		auto device = RenderContext::GetDevice();
		vk::SamplerCreateInfo samplerInfo;
		samplerInfo.flags = vk::SamplerCreateFlags();
		samplerInfo.minFilter = min;
		samplerInfo.magFilter = mag;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;

		samplerInfo.anisotropyEnable = false;
		samplerInfo.maxAnisotropy = 1.0f;

		samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		samplerInfo.unnormalizedCoordinates = false;
		samplerInfo.compareEnable = false;
		samplerInfo.compareOp = vk::CompareOp::eAlways;

		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = (float)0;
		return  device->GetDevice().createSampler(samplerInfo);
	}

	vk::Image Image::CreateVKImage(ImageSpecs& specs, vk::SampleCountFlagBits samples, int mipmap )
	{
		auto device = RenderContext::GetDevice();
		vk::ImageCreateInfo imageInfo;
		imageInfo.flags = vk::ImageCreateFlagBits() | specs.flags;
		imageInfo.imageType = specs.imageType;
		imageInfo.extent = vk::Extent3D(specs.width, specs.height, specs.depth);
		imageInfo.mipLevels = mipmap;
		imageInfo.arrayLayers = 1;
		imageInfo.format = specs.format;
		imageInfo.tiling = specs.tiling;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = specs.usage;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.samples = samples;
		imageInfo.arrayLayers = specs.arrayCount;
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
	std::mutex mutex;

	void Image::UpdateRegionWithImage(std::string path, SPtr<Image> parentImage, vk::Offset3D offset )
	{
		ZoneScopedN("UpdateRegionWithImage");
		stbi_set_flip_vertically_on_load(false);
		int width = 0, height = 0, channels = 0;
		auto pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		assert(width != 0 && height != 0);
		if (!pixels) {
			Log::GetLog()->error("Unable to load: {0}", path);
			return;
		}
		auto device = RenderContext::GetDevice();
	
		auto imageSize = width * height * 4;
		auto buffer = Buffer::CreateStagingBuffer(imageSize);

		//...then fill it,
		void* writeLocation = device->GetDevice().mapMemory(buffer->GetMemory(), 0, imageSize);
		memcpy(writeLocation, pixels, imageSize);
		device->GetDevice().unmapMemory(buffer->GetMemory());



		std::lock_guard<std::mutex> guard{ mutex };
		auto commandBuffer = CommandBuffer::CreateBuffer(parentImage->m_CommandPool, vk::CommandBufferLevel::ePrimary);

		commandBuffer.BeginTransfering();
		commandBuffer.CopyBufferToImage(*buffer.get(), parentImage->m_Image,width, height, 0, offset);
		commandBuffer.EndTransfering();
		commandBuffer.SubmitSingle();

		commandBuffer.Free();
		free(pixels);


	}

	SPtr<Image> Image::CreateImage(std::string path)
	{
		
		auto image = CreateUPtr<Image>();
		
		image->m_CommandPool = Renderer::Instance()->GetCommandPoolManager()->GetFreePool();
		stbi_set_flip_vertically_on_load(true);
		auto pixels = stbi_load(path.c_str(), &image->m_Width, &image->m_Height, &image->m_Channels, STBI_rgb_alpha);
		if (!pixels) {
			Log::GetLog()->error("Unable to load: {0}", path);
		}

		auto mipMaps = static_cast<uint32_t>(std::floor(std::log2(std::max(image->m_Width, image->m_Height)))) + 1;
	
		image->m_Size = image->m_Width * image->m_Height * FormatToSize(vk::Format::eR8G8B8A8Unorm);
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
		commandBuffer.CopyBufferToImage(*buffer.get(), image->m_Image, image->m_Width, image->m_Height);
		commandBuffer.EndTransfering();
		commandBuffer.SubmitSingle();
		
		
		
		//commandBuffer.BeginTransfering();
		//commandBuffer.ChangeImageLayout(image.get(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, mipMaps);
		//commandBuffer.EndTransfering();
		//commandBuffer.SubmitSingle();

		image->GenerateMipmaps(image->m_Image, (VkFormat)image->m_Format, image->m_Width, image->m_Height,mipMaps);


		commandBuffer.Free();
		free(pixels);


		image->m_ImageView = CreateImageView(image->m_Image, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, vk::ImageViewType::e2D, mipMaps);


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
	SPtr<Image> Image::CreateCubemap(std::vector<std::string> pathes)
	{
		auto image = CreateUPtr<Image>();

		image->m_CommandPool = Renderer::Instance()->GetCommandPoolManager()->GetFreePool();
		stbi_set_flip_vertically_on_load(true);
		stbi_uc* pixels[6];

		for (int i = 0; i < 6; i++)
		{
			pixels[i] = stbi_load(pathes[i].c_str(), &image->m_Width, &image->m_Height, &image->m_Channels, STBI_rgb_alpha);
			if (!pixels[i]) {
				Log::GetLog()->error("Unable to load: {0}", pathes[i]);
			}
		}

		auto mipMaps = static_cast<uint32_t>(std::floor(std::log2(std::max(image->m_Width, image->m_Height)))) + 1;
		mipMaps = 1;
		image->m_Size = image->m_Width * image->m_Height * FormatToSize(vk::Format::eR8G8B8A8Unorm);
		auto device = RenderContext::GetDevice();
		ImageSpecs specs;
		specs.width = image->m_Width;
		specs.height = image->m_Height;
		specs.format = vk::Format::eR8G8B8A8Unorm;
		specs.tiling = vk::ImageTiling::eOptimal;
		specs.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
		specs.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		specs.arrayCount = 6;
		specs.flags = vk::ImageCreateFlagBits::eCubeCompatible;
		image->m_Format = specs.format;
		try {
			image->m_Image = CreateVKImage(specs, vk::SampleCountFlagBits::e1, mipMaps);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Unable to make cube image");
		}

		try
		{
			image->m_ImageMemory = CreateMemory(image->m_Image, specs);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Unable to make memory for cube image");
		}

		// populate memory with data
		auto imageSize = image->m_Width * image->m_Height * 4;
		auto buffer = Buffer::CreateStagingBuffer(6*imageSize);




		//...then fill it,
		for (int i = 0; i < 6; ++i) {
			void* writeLocation = device->GetDevice().mapMemory(buffer->GetMemory(), imageSize * i,  imageSize);
			memcpy(writeLocation, pixels[i], imageSize);
			device->GetDevice().unmapMemory(buffer->GetMemory());
		}



		//then transfer it to image memory
		auto commandBuffer = CommandBuffer::CreateBuffer(image->m_CommandPool, vk::CommandBufferLevel::ePrimary);


		commandBuffer.BeginTransfering();
		commandBuffer.ChangeImageLayout(image.get(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipMaps,6);
		commandBuffer.EndTransfering();
		commandBuffer.SubmitSingle();

		commandBuffer.BeginTransfering();
		commandBuffer.CopyBufferToImage(*buffer.get(), image->m_Image, image->m_Width, image->m_Height, 0, vk::Offset3D{ 0,0,0 }, 6);
		commandBuffer.EndTransfering();
		commandBuffer.SubmitSingle();



		commandBuffer.BeginTransfering();
		commandBuffer.ChangeImageLayout(image.get(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, mipMaps,6);
		commandBuffer.EndTransfering();
		commandBuffer.SubmitSingle();

		//image->GenerateMipmaps(image->m_Image, (VkFormat)image->m_Format, image->m_Width, image->m_Height, mipMaps);


		commandBuffer.Free();
		for (int i = 0; i < 6; ++i) {
			free(pixels[i]);
		}


		image->m_ImageView = CreateImageView(image->m_Image, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, vk::ImageViewType::eCube, mipMaps,6);


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
			image->m_Sampler = device->GetDevice().createSampler(samplerInfo);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to make sampler for cube image");

		}




		return image;
	}
	SPtr<Image> Image::CreateEmptyImage(int width, int height, vk::Format format, vk::ImageUsageFlags usage,int mipLevels, vk::SampleCountFlagBits samples, vk::Filter minFilter, vk::Filter magFilter)
	{
		auto image = CreateUPtr<Image>();
		image->m_CommandPool = Renderer::Instance()->GetCommandPoolManager()->GetFreePool();
		image->m_Width = width;
		image->m_Height = height;

		image->m_Size = image->m_Width * image->m_Height * FormatToSize(format);
		auto device = RenderContext::GetDevice();
		ImageSpecs specs;
		specs.width = image->m_Width;
		specs.height = image->m_Height;
		specs.format = format;
		specs.tiling = vk::ImageTiling::eOptimal;
		specs.usage = usage;
		specs.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		image->m_Format = specs.format;
		try {
			image->m_Image = CreateVKImage(specs, samples, mipLevels);
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













		image->m_ImageView = CreateImageView(image->m_Image, format, vk::ImageAspectFlagBits::eColor);


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
		samplerInfo.minFilter = minFilter;
		samplerInfo.magFilter = magFilter;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;

		samplerInfo.anisotropyEnable = false;
		samplerInfo.maxAnisotropy = 1.0f;

		samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		samplerInfo.unnormalizedCoordinates = false;
		samplerInfo.compareEnable = false;
		samplerInfo.compareOp = vk::CompareOp::eAlways;

		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
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

	void Image::Fill(glm::vec4 value, CommandBuffer& cmd, SPtr<Buffer> stageBuffer, int bufferOffset)
	{
		auto ptr = RenderContext::GetDevice()->GetDevice().mapMemory(stageBuffer->GetMemory(), 0, m_Size);
		std::vector<glm::vec4> values(m_Width * m_Height, value);
		memcpy(ptr, values.data(), m_Size);
		RenderContext::GetDevice()->GetDevice().unmapMemory(stageBuffer->GetMemory());
		auto prevLayout = m_ImageLayout;
		cmd.MemBufferBarrier(stageBuffer->GetBuffer(), stageBuffer->GetSize(),
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::AccessFlagBits::eNone,
			vk::PipelineStageFlagBits::eComputeShader,
			vk::AccessFlagBits::eShaderWrite);
		cmd.ChangeImageLayout(this, m_ImageLayout, vk::ImageLayout::eTransferDstOptimal);
		cmd.CopyBufferToImage(*stageBuffer, m_Image, m_Width, m_Height, bufferOffset);
		cmd.ChangeImageLayout(this, m_ImageLayout, prevLayout);
	}
	SPtr<Image> Image::CreateEmpty3DImage(int width, int height,int depth, vk::Format format)
	{
		auto image = CreateUPtr<Image>();

		image->m_CommandPool = Renderer::Instance()->GetCommandPoolManager()->GetFreePool();
		image->m_Width = width;
		image->m_Height = height;
		image->m_Depth = depth;
		image->m_Size = image->m_Width * image->m_Height * FormatToSize(format);

		auto device = RenderContext::GetDevice();
		ImageSpecs specs;
		specs.width = image->m_Width;
		specs.height = image->m_Height;
		specs.depth = image->m_Depth;
		specs.format = format;
		specs.tiling = vk::ImageTiling::eOptimal;
		specs.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
		specs.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		specs.imageType = vk::ImageType::e3D;
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
		auto imageSize = image->m_Width * image->m_Height*image->m_Depth * 4;
		auto buffer = Buffer::CreateStagingBuffer(imageSize);













		image->m_ImageView = CreateImageView(image->m_Image, format, vk::ImageAspectFlagBits::eColor,vk::ImageViewType::e3D);


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
		// might be swapchain image
		if (m_ImageMemory.operator!=(VK_NULL_HANDLE))
		{
			logicalDevice.freeMemory(m_ImageMemory);
		}
		if (m_Image.operator!=(VK_NULL_HANDLE))
		{
			logicalDevice.destroyImage(m_Image);
		}
		if (m_Sampler.operator!=(VK_NULL_HANDLE))
		{
			logicalDevice.destroySampler(m_Sampler);
		}

		logicalDevice.destroyImageView(m_ImageView);
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
	void Image::GenerateMipmaps(uint32_t mipLevels)
	{
		
		GenerateMipmaps(m_Image, (VkFormat)m_Format, (int32_t)m_Width, (int32_t)m_Height, mipLevels);

	}
	std::vector<SPtr<Image>> Image::GenerateEmptyMipmapsAsImages(uint32_t mipLevels)
	{
		std::vector<SPtr<Image>> images;
		int32_t mipWidth = 1;
		int32_t mipHeight = 1;
		auto transfer = Renderer::Instance()->GetTransferCommandBuffer(0);
		transfer.BeginTransfering();
		for (uint32_t i = 0; i < mipLevels; i++)
		{
			
			// create image
			ImageSpecs specs;
			specs.width = mipWidth;
			specs.height = mipHeight;
			specs.format = m_Format;
			specs.tiling = vk::ImageTiling::eOptimal;
			specs.usage =  vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
			specs.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;

			auto vkImage = CreateVKImage(specs,vk::SampleCountFlagBits::e1);
			//create memory
			
			auto memory = CreateMemory(vkImage, specs);


			//  create view
			auto view = CreateImageView(vkImage, m_Format, vk::ImageAspectFlagBits::eColor, vk::ImageViewType::e2D);
			// create sampler

			auto image = CreateSPtr<Image>();
			image->SetFormat(m_Format);
			image->SetMemory(memory);
			image->SetView(view);
			image->SetImage(vkImage);
			image->m_Width = mipWidth;
			image->m_Height = mipHeight;
			image->m_Size = image->m_Width * image->m_Height * FormatToSize(m_Format);
			image->m_Sampler  = CreateSampler(vk::Filter::eNearest, vk::Filter::eNearest);
			transfer.ChangeImageLayout(image.get(), vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
			images.push_back(image);
			mipHeight *= 2;
			mipWidth *= 2;
		}
		transfer.EndTransfering();
		transfer.SubmitSingle();
		return images;
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