#include"Prereq.h"
#include"RenderContext.h"
#include"Device.h"
#include"Queue.h"
namespace Voidstar
{
	Device* RenderContext::m_Device;
	Queue* RenderContext::m_GraphicsQueue;

	Device* RenderContext::GetDevice()
	{
		return m_Device;
	}
	void RenderContext::SetDevice(Device* device)
	{
		m_Device = device;
	}
	Queue* RenderContext::GetGraphicsQueue()
	{
		return m_GraphicsQueue;
	}
	void RenderContext::SetGraphicsQueue(Queue* queue)
	{
		m_GraphicsQueue = queue;
	}
}