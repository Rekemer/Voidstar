#pragma once
namespace Voidstar
{
	class CommandBuffer;
	struct IExecute
	{
		virtual void Execute(CommandBuffer& cmd, size_t frameIndex) = 0;
		virtual bool IsCompute() { return false; };
		virtual ~IExecute() {};
	};
};