#pragma once
#include"Application.h"

extern Voidstar::Application* Voidstar::CreateApplication();

int Main()
{
	auto app = Voidstar::CreateApplication();
	app->Run();
	delete app;
	return 0;
}



