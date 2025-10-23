#pragma once
#include "Core.h"

namespace Voidstar
{
	// used in descriptor creation too, hence bit operations
	enum class VOIDSTAR_API ShaderType : uint32_t
	{
		NONE = 0,
		VERTEX = 1 << 0,
		FRAGMENT = 1 << 1,
		COMPUTE = 1 << 2,
		TESS_CONTROL = 1 << 3,
		TESS_EVALUATION = 1 << 4,
		ALL = VERTEX | FRAGMENT | COMPUTE | TESS_CONTROL | TESS_EVALUATION,
	};


	inline bool HasFlag(ShaderType value, ShaderType flag)
	{
		return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
	}

	inline ShaderType operator|(ShaderType a, ShaderType b)
	{
		return static_cast<ShaderType>(
			static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

	inline ShaderType& operator|=(ShaderType& a, ShaderType b)
	{
		a = a | b;
		return a;
	}

	inline ShaderType operator&(ShaderType a, ShaderType b)
	{
		return static_cast<ShaderType>(
			static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}

}