#include "ModelSandbox.h"
#include "DOS.h"

Voidstar::Application* Voidstar::CreateApplication()
{
	// 120 -> 1920 * 1080
	// 110
	const int res = 50;
	return new ModelSandbox( "Voidstar Demo ", std::min(16 * res, 1920), std::min(9 * res, 1061));
	//return new DOS( "Voidstar Demo ", std::min(16 * res, 1920), std::min(9 * res, 1061));
}

int main()
{
	return Main();
}