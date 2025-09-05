#pragma once
#include<string_view>
namespace Voidstar 
{
	struct InitParams
	{
		char appName[256];
		size_t width, height;
		InitParams() = default;

		InitParams(size_t width, size_t height)
		{
			this->width = width;
			this->height= height;
			
			std::string_view name = "example";
			std::memcpy(appName, name.data(), name.size());
			appName[name.size()] = '\0';
		}

		void SetName(std::string_view str)
		{
			size_t n = std::min(str.size(), sizeof(appName) - 1);
			std::memcpy(appName, str.data(), n);
			appName[n] = '\0';
		}
	};
}