#pragma once
#include <string_view>
#include <unordered_map>
#include "ShaderType.h"
#include "Reflection.h"
namespace Voidstar
{
	class ShaderCompiler
	{
	public:
		void Init();
		void Compile(std::string_view path, ShaderType type);
	private:
		std::unordered_map<ProgramHandle, ProgramMeta> m_Programs;
	};
}