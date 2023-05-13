

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
	return new ExampleApplication(str, 16, 9);
}
int main()
{
	return Main();
}
