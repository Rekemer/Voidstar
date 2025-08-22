#pragma once
#include "Core.h"

namespace Voidstar
{
	enum class VOIDSTAR_API ShaderType : uint32_t
	{
		VERTEX,
		FRAGMENT,
		COMPUTE,
		TESS_CONTROL,
		TESS_EVALUATION
	};
}