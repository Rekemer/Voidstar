#include "Prereq.h"
#include "Submission.h"

namespace Voidstar
{
	class Frame
	{
		
		// command to execute before Render/Compute API calls
		ResourceCommandBuffer m_CmdPre;
		// command to execute after Render/Compute API calls
		ResourceCommandBuffer m_CmdPost;
	};

	Frame m_frame[1];
	// the one doing API calls
	Frame* m_render;
	// the one registering user commands
	Frame* m_submit;
}


void Voidstar::Submit(PassID id, ProgramHandle)
{
	
}

void Voidstar::Init()
{
	m_submit = m_render = &m_frame[0];
}

void Voidstar::Render()
{

}