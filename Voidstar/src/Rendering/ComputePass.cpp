#include"Prereq.h"
#include"ComputePass.h"
#include"Pipeline.h"
namespace Voidstar
{
	void ComputePass::Execute(CommandBuffer& cmd, size_t frameIndex)
	{
		m_Exec(cmd, frameIndex);
	}
}