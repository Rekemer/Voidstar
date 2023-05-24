#include"Prereq.h"
#include"RenderContext.h"
#include"Device.h"
namespace Voidstar
{
	Device* RenderContext::m_Device;

	Device* RenderContext::GetDevice()
	{
		return m_Device;
	}
	void RenderContext::SetDevice(Device* device)
	{
		m_Device = device;
	}
}