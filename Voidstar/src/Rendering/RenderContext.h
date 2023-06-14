#pragma once
namespace Voidstar
{
	class Device;
	class Queue;
	class RenderContext
	{
	public:
		static Device* GetDevice();
		static void SetDevice(Device* device);

		static Queue* GetGraphicsQueue();
		static  void SetGraphicsQueue(Queue* device);
	private:
		static Device* m_Device;
		static Queue* m_GraphicsQueue;
	};
}