#include"Prereq.h"
#include "Quadtree.h"
#include <bitset>
namespace Voidstar
{
	float levelOfDetail = 3;
	// level equals amount of tiles on grid
// level determines size of each coordinates in bite level = amount of bits per coordinates
	constexpr size_t size = 64;
#define F_32 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
#define F_16 0xFFFFFFFFFFFFFFFF
	std::bitset<size> codeQuadtreeNode(std::bitset<size> x, std::bitset<size> y, int level) {
		std::bitset<size> quadtreeCode;
		std::cout << "x " << x << "\n";
		std::cout << "y " << y << "\n";
		quadtreeCode |= x << level | y;
		return quadtreeCode;
	}

	void GetCoords(std::bitset<size> quadCode, uint64_t level)
	{
		std::bitset<size> x;
		std::bitset<size> y;
		std::bitset<size> mask = F_16 >> (128 - level);
		std::cout << "mask for y coordiante " << mask << std::endl;
		y = quadCode & mask;
		x = quadCode >> level;
		std::cout << "\nx " << x.to_ulong() << std::endl;
		std::cout << " y " << y.to_ulong() << std::endl;
	}
	//int main() {
	//	uint64_t x = 63; // x-coordinate of the node
	//	uint64_t y = 61; // y-coordinate of the node
	//	uint64_t level = 6; // level of the node
	//	// 10 01 
	//
	//	auto bits = codeQuadtreeNode(x, y, level);
	//	std::cout << "\nBit of number " << bits << std::endl;
	//	GetCoords(bits, level);
	//	return 0;
	//}

	Quadtree Quadtree::Build( glm::vec2& posPlayer)
	{
		Quadtree result;
		auto min = glm::vec2{ -20,-20 };
		auto max = glm::vec2{ 20,20 };
		Box box{min,max};
		result.BuildTree();
		return result;
	}
	Node& Quadtree::GetLeft(Node& currentNode)
	{
		// TODO: вставьте здесь оператор return
	}
	Node& Quadtree::GetRight(Node& currentNode)
	{
		// TODO: вставьте здесь оператор return
	}
	Node& Quadtree::GetUp(Node& currentNode)
	{
		// TODO: вставьте здесь оператор return
	}
	Node& Quadtree::GetBottom(Node& currentNode)
	{
		// TODO: вставьте здесь оператор return
	}
	void Quadtree::Clear(Node& node)
	{
	}
	void Quadtree::GenerateChildren(Node& node)
	{
		float tileScale = node.tileWidth;
		auto centerOfParentTile = node.worldPosition;
		glm::vec3 leftTop;
		leftTop.x = centerOfParentTile.x + tileScale / 2;
		leftTop.y = 0.f;
		leftTop.z = centerOfParentTile.z + tileScale / 2;
		glm::vec3 rightTop;

		rightTop.x = centerOfParentTile.x - tileScale / 2;
		rightTop.y = 0.f;
		rightTop.z = centerOfParentTile.z + tileScale / 2;

		glm::vec3 leftBottom;

		leftBottom.x = centerOfParentTile.x + tileScale / 2;
		leftBottom.y = 0.f;
		leftBottom.z = centerOfParentTile.z - tileScale / 2;


		glm::vec3 rightBottom;


		rightBottom.x = centerOfParentTile.x - tileScale / 2;
		rightBottom.y = 0.f;
		rightBottom.z = centerOfParentTile.z - tileScale / 2;
		//nodes.emplace_back(leftTop, tileScale, depth);
		//nodes.emplace_back(rightTop, tileScale, depth);
		//nodes.emplace_back(leftBottom, tileScale, depth);
		//nodes.emplace_back(rightBottom, tileScale, depth);
	}
	void Quadtree::BuildTree()
	{

	}

}