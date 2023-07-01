#pragma once
namespace Voidstar
{
	class Device;
	class CommandBuffer;
	class RenderContext
	{
	public:
		static Device* GetDevice();
		static void SetDevice(Device* device);

		static CommandBuffer* GetGraphicsCommandBuffer();
		static  void SetGraphicsCommandBuffer(CommandBuffer* device);
	private:
		static Device* m_Device;
		static CommandBuffer* m_GraphicsCommandBuffer;
	};
}