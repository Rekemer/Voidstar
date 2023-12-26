#pragma once
#include"unordered_map"
#include"vulkan/vulkan.hpp"
class FramebufferManager
{
	
	
	
	/*vk::Framebuffer& GetFramebuffer(std::vector<std::string_view>&  names)
	{
		
		return m_Framebuffers.at(names.data());
	}*/

private:
	std::unordered_map<std::string, vk::Framebuffer>m_Framebuffers;
};