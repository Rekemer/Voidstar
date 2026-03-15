#include "ModelSandbox.h"
#include "DOS.h"

Voidstar::Application* Voidstar::CreateApplication()
{
	auto str = std::string("ModelSandbox");
	// 120 -> 1920 * 1080
	// 110
	const int res = 120;
	return new DOS(str, std::min(16 * res, 1920), std::min(9 * res, 1061));
}

int main()
{
	return Main();
}