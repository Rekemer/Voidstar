#pragma once
#pragma once
#include "Voidstar.h"
#include "Vertex.h"
using namespace Voidstar;

class DOS : public Voidstar::Application
{
public:
	DOS(std::string appName, size_t screenWidth, size_t screenHeight);
	void Update(float deltaTime) override;

};
