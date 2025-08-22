#pragma once
#include <string_view>
#include "Rendering/ShaderType.h"
namespace Voidstar 
{
	using ProgramHandle = uint32_t;
	using ShaderHandle = uint32_t;
	using PassID = uint16_t;

	struct ProgramDesc {
		ShaderHandle vs{}, tcs{}, tes{}, gs{}, fs{};
		ShaderHandle cs{};             
	};

	ProgramHandle LoadShader(std::string_view vertex, std::string_view fragment);

	ProgramHandle LoadShader(std::string_view shader, ShaderType type);
}