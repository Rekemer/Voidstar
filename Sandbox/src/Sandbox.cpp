

#include <iostream>
#include"Voidstar.h"

using namespace Voidstar;


class ExampleApplication : public Application
{
public:
	ExampleApplication(std::string appName, size_t screenWidth, size_t screenHeight) : Application(appName, screenWidth, screenHeight)
	{

	}
		
};


Voidstar::Application* Voidstar::CreateApplication()
{
	auto str = std::string("Example");
	// 120 -> 1920 * 1080
	// 110
	const int res = 90;
	return new ExampleApplication(str, 16 * res, 9 * res);
}
int main()
{
	return Main();
}
