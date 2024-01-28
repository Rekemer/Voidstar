#pragma once
#include"IExecute.h"
#include"../Types.h"
#include"vector"
#include"vulkan/vulkan.hpp"
namespace Voidstar
{
	class CommandBuffer;
	class Pipeline;
	class ComputePass : public IExecute
	{
	public:
		ComputePass(Func exec)  : m_Exec{ exec } {

		}
		void Execute(CommandBuffer& cmd, size_t frameIndex) override;
		bool IsCompute() override { return true; };

	private:
		Func m_Exec;

	};
};