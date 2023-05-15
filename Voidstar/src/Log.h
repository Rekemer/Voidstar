#pragma once
#include"Prereq.h"
#include"spdlog.h"
namespace Voidstar
{
	class Log
	{
	public:

		static void Init();

		static std::shared_ptr<spdlog::logger> GetLog()
		{
			return s_Logger;
		}
	private:
		static std::shared_ptr<spdlog::logger> s_Logger;
	};

}

