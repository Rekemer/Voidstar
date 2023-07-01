#include"Prereq.h"
#include"RenderContext.h"
#include"Device.h"
#include"CommandBuffer.h"
namespace Voidstar
{
	Device* RenderContext::m_Device;
	CommandBuffer* RenderContext::m_GraphicsCommandBuffer;

	Device* RenderContext::GetDevice()
	{
		return m_Device;
	}
	void RenderContext::SetDevice(Device* device)
	{
		m_Device = device;
	}
	CommandBuffer* RenderContext::GetGraphicsCommandBuffer()
	{
		return m_GraphicsCommandBuffer;
	}
	void RenderContext::SetGraphicsCommandBuffer(CommandBuffer* CommandBuffer)
	{
		m_GraphicsCommandBuffer = CommandBuffer;
	}
}