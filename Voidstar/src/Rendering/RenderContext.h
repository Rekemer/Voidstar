#pragma once
namespace Voidstar
{
	class Device;
	class RenderContext
	{
	public:
		static Device* GetDevice();
		static void SetDevice(Device* device);
	private:
		static Device* m_Device;
	};
}