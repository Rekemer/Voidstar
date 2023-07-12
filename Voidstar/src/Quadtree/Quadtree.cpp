#include"Prereq.h"
#include "Quadtree.h"


namespace Voidstar
{


	const float groundSize = 10;
	const int widthGround = 2;
	const int heightGround = 2;

	float levelOfDetail = 6;




	// 2 bits per depth
	std::bitset<size> ChildOfParent(int x, std::bitset<size> parentIndex, int childrenDepth)
	{
		std::bitset<size> nodeCoords;
		nodeCoords |= parentIndex;

		std::bitset<size> maskCoords;
		maskCoords |= std::bitset<size>(x);
		maskCoords <<= (childrenDepth-1) * 2;
		nodeCoords |= maskCoords;
		std::cout << " newNode of "<< x << "coords "<< parentIndex<< " is" << maskCoords << std::endl;
		return nodeCoords;

	}
	std::bitset<size> ParentOfChild(std::bitset<size> child, int childDepth)
	{
		std::bitset<size> maskCoords;
		for (int i = 0; i < childDepth * 2 - 3; i++)
		{
			maskCoords[i] = 1;
			maskCoords[i + 1] = 1;
		}
		std::cout << " mask " << child << std::endl;
		return child & maskCoords;
	}

	
	Quadtree Quadtree::Build( glm::vec3 posPlayer)
	{
		Quadtree result;
		auto min = glm::vec2{ -20,-20 };
		auto max = glm::vec2{ 20,20 };
		Box box{min,max};
		result.root.tileWidth = groundSize;
		result.root.worldPosition = glm::vec3{0,0,0};

		//result.GenerateChildren(result.root, 1);
		result.BuildTree(posPlayer, result.root,1);
		return result;
	}
	
	Node Quadtree::GetNode(std::bitset<size> node, int depth)
	{
		auto& vector = nodes[depth];
		for (int i = 0; i < vector.size(); i++)
		{
			if (vector[i].index == node)
			{
				return vector[i];
			}
		}
		return{};
	}


	//depth 1 = 2
	//depth 2 = 4
	//depth 3 = 5
	std::bitset<size> GetSibling(Coordinate coord, std::bitset<size> node, int depthOfNode)
	{
		if ((depthOfNode * 2 - 1) <= 0)
		{
			return {};
		}
		std::bitset<size> temp{ coord };
		node[depthOfNode * 2 - 1] = temp[1];
		node[depthOfNode*2 -2] = temp[2];
		return node;
	}

	std::optional<Node> Quadtree::GetLeft(Coordinate coord,std::bitset<size> node,int depthOfNode)
	{
		

		auto siblingIndex = GetSibling(coord, node, depthOfNode);
		return GetNode(siblingIndex, depthOfNode);
	}

	void Quadtree::Clear(Node& node)
	{
	}

	

	GeneratedChildren Quadtree::GenerateChildren(Node& node,int depth)
	{

		//auto iterator = std::find(nodes[depth].begin(), nodes[depth].end(), node);
		//if (iterator != nodes.end())
		//{
		//	nodes[depth].erase(iterator);
		//}

		float tileScale = node.tileWidth/2;
		auto centerOfParentTile = node.worldPosition;

		Node leftTop;
		leftTop.worldPosition.x = centerOfParentTile.x + tileScale / 2;
		leftTop.worldPosition.y = 0.f;
		leftTop.worldPosition.z = centerOfParentTile.z + tileScale / 2;
		leftTop.index = ChildOfParent(0,node.index, depth);
		leftTop.tileWidth = tileScale;
		Node rightTop;

		rightTop.worldPosition.x = centerOfParentTile.x - tileScale / 2;
		rightTop.worldPosition.y = 0.f;
		rightTop.worldPosition.z = centerOfParentTile.z + tileScale / 2;
		rightTop.index = ChildOfParent(1, node.index, depth);
		rightTop.tileWidth = tileScale;
		Node leftBottom;

		leftBottom.worldPosition.x = centerOfParentTile.x + tileScale / 2;
		leftBottom.worldPosition.y = 0.f;
		leftBottom.worldPosition.z = centerOfParentTile.z - tileScale / 2;
		leftBottom.index = ChildOfParent(2, node.index, depth);
		leftBottom.tileWidth = tileScale;

		Node rightBottom;


		rightBottom.worldPosition.x = centerOfParentTile.x - tileScale / 2;
		rightBottom.worldPosition.y = 0.f;
		rightBottom.worldPosition.z = centerOfParentTile.z - tileScale / 2;
		rightBottom.index = ChildOfParent(3, node.index, depth);
		rightBottom.tileWidth = tileScale;

		

		nodes[depth].emplace_back(leftTop);
		nodes[depth].emplace_back(rightTop);
		nodes[depth].emplace_back(leftBottom);
		nodes[depth].emplace_back(rightBottom);
		return  { leftTop ,rightTop,leftBottom,rightBottom };


	}
	void Quadtree::BuildTree(glm::vec3 playerPos,Node& nodeToDivide,int depth)
	{
		glm::vec3 posPlayer = playerPos;
		//auto distance = posPlayer - tilePos;
		bool isDecreaseRes = false;
		auto tilePos = nodeToDivide.worldPosition;

		if (depth >= levelOfDetail)
		{
			return;
		}
		auto children = GenerateChildren(nodeToDivide, depth);
		float tileWidthOfTileToDivide = nodeToDivide.tileWidth;
		const glm::vec3 rightOffset{ -tileWidthOfTileToDivide * 2 ,0,0 };
		const glm::vec3 upOffset{ 0,0,tileWidthOfTileToDivide * 2 };
		const glm::vec3 bottomOffset{ 0,0,-tileWidthOfTileToDivide * 2 };
		const glm::vec3 leftOffset{ tileWidthOfTileToDivide * 2,0,0 };
		const glm::vec3 leftTopOffset{ tileWidthOfTileToDivide * 2,0,tileWidthOfTileToDivide * 2 };
		const glm::vec3 rightTopOffset{ -tileWidthOfTileToDivide * 2,0,tileWidthOfTileToDivide * 2 };
		const glm::vec3 rightBottomOffset{ -tileWidthOfTileToDivide * 2,0,-tileWidthOfTileToDivide * 2 };
		const glm::vec3 leftBottomOffset{ tileWidthOfTileToDivide * 2,0,-tileWidthOfTileToDivide * 2 };


		if (depth != 1)
		{
			//auto nodeLeftTop = GetLeft(0, nodeToDivide.index, depth);
			//auto nodeLeftBottom = GetLeft(3, nodeToDivide.index, depth);

			//auto nodeRight = GetRight(nodeToDivide);
			//auto nodeUp = GetUp(nodeToDivide);
			//auto nodeBottom = GetBottom(nodeToDivide);

			//GenerateChildren(nodeLeft, depth);
			//GenerateChildren(nodeUp,  depth);
			//GenerateChildren(nodeBottom,  depth);
			//GenerateChildren(nodeLeft,  depth);
		}
		


		auto tileLeftTop = tilePos + glm::vec3{ tileWidthOfTileToDivide / 2  ,0,tileWidthOfTileToDivide / 2 };
		auto tileRightTop = tilePos + glm::vec3{ -tileWidthOfTileToDivide / 2  ,0,tileWidthOfTileToDivide / 2 };
		auto tileLeftBottom = tilePos + glm::vec3{ tileWidthOfTileToDivide / 2 ,0,-tileWidthOfTileToDivide / 2 };
		auto tileRightBottom = tilePos + glm::vec3{ -tileWidthOfTileToDivide / 2  ,0,-tileWidthOfTileToDivide / 2 };

		// Calculate distances
		float distLeftTop = glm::distance(posPlayer, tileLeftTop);
		float distRightTop = glm::distance(posPlayer, tileRightTop);
		float distLeftBottom = glm::distance(posPlayer, tileLeftBottom);
		float distRightBottom = glm::distance(posPlayer, tileRightBottom);

		// Find the position closest to posPlayer and get new parent index from generated tiles
		Node closestTilePos;
		if (distLeftTop <= distRightTop && distLeftTop <= distLeftBottom && distLeftTop <= distRightBottom)
		{
			//lefttop
			closestTilePos= children.leftTop;
		}
		else if (distRightTop <= distLeftTop && distRightTop <= distLeftBottom && distRightTop <= distRightBottom)
		{
			//righttop
			closestTilePos= children.rightTop;
		}
		else if (distLeftBottom <= distLeftTop && distLeftBottom <= distRightTop && distLeftBottom <= distRightBottom)
		{
			//left bottom
			closestTilePos = children.leftBottom;
		}
		else
		{
			//right bottom
			closestTilePos = children.rightBottom;
		}
		BuildTree(playerPos, closestTilePos,++depth);
	}

}