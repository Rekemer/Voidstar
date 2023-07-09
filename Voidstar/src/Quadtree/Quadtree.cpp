#include"Prereq.h"
#include "Quadtree.h"
namespace Voidstar
{
	Quadtree Quadtree::Build( glm::vec2& posPlayer)
	{
		Quadtree result;
		auto min = glm::vec2{ -20,-20 };
		auto max = glm::vec2{ 20,20 };
		Box box{min,max};
		result.BuildTree();
		return result;
	}
	void Quadtree::BuildTree()
	{

	}

}