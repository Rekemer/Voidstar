#include "Prereq.h"
#include "AttachmentManager.h"
#include "RenderContext.h"
#include "SupportStruct.h"
#include "Device.h"
#include "Renderer.h"

namespace Voidstar
{

	void AttachmentManager::Init(const std::vector<TextureHandle>& handles, AttachmentHandle handle)
	{
		// default can be used as resolve after MSAA
		m_Resolve[handle] = handles;
		// or as direct render targert
		m_Color[handle] = handles;

	}

	void AttachmentManager::CreateColor(AttachmentHandle attachmentName,
		vk::Format format, size_t width, size_t height,
		vk::SampleCountFlagBits samples,
		vk::ImageUsageFlags usage, size_t attachmentAmount, vk::MemoryPropertyFlags flags)
	{
		
		AttachmentSpec msaa;
		msaa.Specs.width = width;
		msaa.Specs.height = height;
		msaa.Specs.usage = usage;
		msaa.Specs.format = format;
		msaa.Specs.minFilter = vk::Filter::eLinear;
		msaa.Specs.magFilter = vk::Filter::eLinear;
		msaa.Specs.tiling = vk::ImageTiling::eOptimal;
		msaa.Specs.memoryProperties = flags;
		msaa.Amount = attachmentAmount;
		msaa.Samples = samples;

		auto specs = msaa.Specs;
		//then transfer it to image memory
		auto commandBuffer = Renderer::Instance()->GetTransferCommandBuffer(Renderer::Instance()->m_CurrentFrame);


		commandBuffer.BeginTransfering();
		std::vector<TextureHandle> images;
		images.resize(msaa.Amount);
		for (int i = 0; i < images.size(); i++)
		{
			auto msaaImage = Image::CreateVKImage(specs, msaa.Samples);
			auto msaaImageMemory = Image::CreateMemory(msaaImage, specs);
			auto msaaImageView = Image::CreateImageView(msaaImage, specs.format, specs.imageAspect);
			images[i] = GetTextureHandle_();
			auto& image = Renderer::Instance()->m_Textures[images[i]];
			image = CreateSPtr<Image>();
			if (usage & vk::ImageUsageFlagBits::eSampled)
			{
				image->m_Sampler = Image::CreateSampler(vk::Filter::eLinear, vk::Filter::eLinear);
			}
			image->m_CommandPool = Renderer::Instance()->GetCommandPoolManager()->GetFreePool();
			image->SetFormat(specs.format);
			image->SetSample(msaa.Samples);
			image->SetView(msaaImageView);
			image->SetMemory(msaaImageMemory);
			image->SetImage(msaaImage);
			image->SetWidth(width);
			image->SetHeight(height);
			image->m_Size = GetImageSize(image->m_Width, image->m_Height, specs.format);
			commandBuffer.ChangeImageLayout(image.get(), vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 1);
		}
		commandBuffer.EndTransfering();
		commandBuffer.SubmitSingle();
		m_Color[attachmentName] = images;
	}
	void AttachmentManager::CreateDepthStencil(AttachmentHandle attachmentName,
		 size_t width, size_t height,
		vk::SampleCountFlagBits samples,
		vk::ImageUsageFlags usage,size_t attachmentAmount)
	{
		DepthStencilSpecs depthSpec;
		depthSpec.Candidates = { vk::Format::eD32SfloatS8Uint ,
			vk::Format::eD24UnormS8Uint,
		vk::Format::eD16UnormS8Uint };
		//{ vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint }
		depthSpec.Specs.width = width;
		depthSpec.Specs.height = height;
		depthSpec.Specs.usage = usage;
		depthSpec.Specs.tiling = vk::ImageTiling::eOptimal;
		depthSpec.Specs.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		depthSpec.Amount = attachmentAmount;
		depthSpec.Samples = samples;
		depthSpec.FormatFeature = vk::FormatFeatureFlagBits::eDepthStencilAttachment;
		depthSpec.Specs.imageAspect = vk::ImageAspectFlagBits::eDepth;

		auto& depth = static_cast<DepthStencilSpecs&>(depthSpec);
		auto device = RenderContext::GetDevice();

		auto depthFormat = Image::GetFormat(
			device->GetDevicePhys(),
			depth.Candidates,
			depth.Specs.tiling,
			depth.FormatFeature
		);
		std::vector<TextureHandle> images;
		images.resize(depth.Amount);
		depth.Specs.format = depthFormat;
		for (int i = 0; i < depth.Amount; i++)
		{
			images[i] = GetTextureHandle_();
			auto& image = Renderer::Instance()->m_Textures[images[i]];
			image = CreateSPtr<Image>();
			auto vkImage = Image::CreateVKImage(depth.Specs, depth.Samples);
			auto imageMemory = Image::CreateMemory(vkImage, depth.Specs);
			auto imageView = Image::CreateImageView(vkImage, depthFormat, depth.Specs.imageAspect);
			image = CreateSPtr<Image>();
			image->SetFormat(depth.Specs.format);
			image->SetSample( depth.Samples);
			image->SetView(imageView);
			image->SetMemory(imageMemory);
			image->SetImage(vkImage);
			image->SetWidth(width);
			image->SetHeight(height);
			
		}
		m_DepthStencil[attachmentName] = images;
	}

	std::vector<SPtr<Image>> AttachmentManager::GetAttachhmentsFrom(std::unordered_map<AttachmentHandle, std::vector<TextureHandle>>& from,
		std::vector<AttachmentHandle> names)
	{
		std::vector<SPtr<Image>> attachments;
		for (auto name : names)
		{
			auto& attachment = from.at(name);
			for (auto& handle : attachment)
			{

				attachments.push_back(Renderer::Instance()->GetTexture(handle));

			}

		}
		return attachments;
	}


	TextureHandle AttachmentManager::GetColorTexture(AttachmentHandle handle, size_t frameNumber)
	{
		return m_Color.at(handle).at(frameNumber);
	}

	void AttachmentManager::Destroy()
	{
		//auto device = RenderContext::GetDevice()->GetDevice();
		//device.waitIdle();
		//for (auto& e : m_Color)
		//{
		//	std::for_each(e.second.begin(),
		//		e.second.end(),
		//		[](SPtr<Image>& image)
		//		{
		//			image.reset();
		//		});
		//}
		//for (auto& e : m_DepthStencil)
		//{
		//
		//	std::for_each(e.second.begin(),
		//		e.second.end(),
		//		[](SPtr<Image>& image)
		//		{
		//			image.reset();
		//		});
		//}
		//for (auto& e : m_Resolve)
		//{
		//
		//	std::for_each(e.second.begin(),
		//		e.second.end(),
		//		[](SPtr<Image>& image)
		//		{
		//			image.reset();
		//		});
		//}
	}
	
}