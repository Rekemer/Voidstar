#include"Prereq.h"
#include "Quadtree.h"
#include "Log.h"


namespace Voidstar
{


	const float groundSize = 10;
	const int widthGround = 2;
	const int heightGround = 2;

	float levelOfDetail =3;




	// 2 bits per depth
	std::bitset<size> ChildOfParent(int x, std::bitset<size> parentIndex, int childrenDepth)
	{
		std::bitset<size> nodeCoords;
		nodeCoords |= parentIndex;

		std::bitset<size> maskCoords;
		maskCoords |= std::bitset<size>(x);
		maskCoords <<= (childrenDepth-1) * 2;
		nodeCoords |= maskCoords;
		//std::cout << " newNode of "<< x << "coords "<< parentIndex<< " is" << maskCoords << std::endl;
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
		result.root.isDrawn = false;
		result.root.worldPosition = glm::vec3{0,0,0};



		float currentTileWidth = groundSize / static_cast<float>(widthGround);
		float currentTileHeight = groundSize / static_cast<float>(heightGround); ;
		glm::vec3 centerOffset = { currentTileWidth /2,0.,currentTileHeight /2 };
		glm::vec3 currentTilePosBiggest = {0,0,0};
		bool isBiggestFound = false;
		uint32_t index = 0;
		float shortestPath = 1000;
		Node currentNode;
		

		//result.GenerateChildren(result.root, 1);
		result.BuildTree(posPlayer, result.root,1);
		return result;
	}
	
	Node& Quadtree::GetNode(std::bitset<size> node, int depth)
	{
		auto& vector = nodes[depth];
		for (int i = 0; i < vector.size(); i++)
		{
			if (vector[i].index == node)
			{
				return vector[i];
			}
		}
		return{root};
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
		node[depthOfNode*2 -2] = temp[0];
		return node;
	}

	
	std::optional<Node> Quadtree::GetLeft(Coordinate coord,std::bitset<size> node,int depthOfNode)
	{
		

		auto siblingIndex = GetSibling(coord, node, depthOfNode);

		if (siblingIndex == node)
		{
		//{
		//auto parentDepth = depthOfNode - 1;
		//auto parent = ParentOfChild(node,depthOfNode);
		//auto parentSibling = GetSibling(1,parent,parentDepth);
		return {};
		}

		return GetNode(siblingIndex, depthOfNode);
	}

	void Quadtree::Clear(Node& node)
	{
	}

	

	GeneratedChildren Quadtree::GenerateChildren(Node& node,int depth)
	{

		
		//auto parentDepth = depth - 1;
		//if (parentDepth != 0)
		//{
		auto& node1 = GetNode(node.index, depth - 1);
		node1.isDrawn = false;
		//}
		
		float tileScale = node.tileWidth / 2;
		auto centerOfParentTile = node.worldPosition;

		auto childDepth = depth;
		Node leftTop;
		leftTop.worldPosition.x = centerOfParentTile.x + tileScale / 2;
		leftTop.worldPosition.y = 0.f;
		leftTop.worldPosition.z = centerOfParentTile.z + tileScale / 2;
		leftTop.index = ChildOfParent(0, node.index, childDepth);
		leftTop.tileWidth = tileScale;
		Node rightTop;

		rightTop.worldPosition.x = centerOfParentTile.x - tileScale / 2;
		rightTop.worldPosition.y = 0.f;
		rightTop.worldPosition.z = centerOfParentTile.z + tileScale / 2;
		rightTop.index = ChildOfParent(1, node.index, childDepth);
		rightTop.tileWidth = tileScale;
		Node leftBottom;

		leftBottom.worldPosition.x = centerOfParentTile.x + tileScale / 2;
		leftBottom.worldPosition.y = 0.f;
		leftBottom.worldPosition.z = centerOfParentTile.z - tileScale / 2;
		leftBottom.index = ChildOfParent(2, node.index, childDepth);
		leftBottom.tileWidth = tileScale;

		Node rightBottom;


		rightBottom.worldPosition.x = centerOfParentTile.x - tileScale / 2;
		rightBottom.worldPosition.y = 0.f;
		rightBottom.worldPosition.z = centerOfParentTile.z - tileScale / 2;
		rightBottom.index = ChildOfParent(3, node.index, childDepth);
		rightBottom.tileWidth = tileScale;


		nodes[childDepth].emplace_back(leftTop);
		nodes[childDepth].emplace_back(rightTop);
		nodes[childDepth].emplace_back(leftBottom);
		nodes[childDepth].emplace_back(rightBottom);
		return  { leftTop.index,rightTop.index,leftBottom.index,rightBottom.index };
	

	}
	void Quadtree::BuildTree(glm::vec3 playerPos,Node nodeToDivide,int depth)
	{
		//if (nodeToDivide.index == std::bitset<size>(""))
		glm::vec3 posPlayer = playerPos;
		//auto distance = posPlayer - tilePos;
		bool isDecreaseRes = false;
		auto tilePos = nodeToDivide.worldPosition;

		if (depth > levelOfDetail)
		{
			return;
		}
		auto& children = GenerateChildren(nodeToDivide, depth);
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
			auto parentDepth = depth - 1;
			auto nodeLeftTop = GetLeft(0, nodeToDivide.index, parentDepth);
			auto nodeLeftBottom = GetLeft(2, nodeToDivide.index, parentDepth);
			auto nodeRightTop = GetLeft(1, nodeToDivide.index, parentDepth);
			auto nodeRightBottom = GetLeft(3, nodeToDivide.index, parentDepth);
			//auto nodeUp = GetUp(nodeToDivide);
			//auto nodeBottom = GetUp(nodeToDivide);

			if (nodeLeftTop.has_value())
			{
				//Log::GetLog()->info("{0} {1} {2}",nodeLeftTop.value().worldPosition.x , nodeLeftTop.value().worldPosition.y, nodeLeftTop.value().worldPosition.z);
				GenerateChildren(nodeLeftTop.value(), parentDepth+1);
			}
			if (nodeLeftBottom.has_value())
			{
				GenerateChildren(nodeLeftBottom.value(), parentDepth+1);
			}
			if (nodeRightTop.has_value())
			{
				//Log::GetLog()->info("{0} {1} {2}",nodeLeftTop.value().worldPosition.x , nodeLeftTop.value().worldPosition.y, nodeLeftTop.value().worldPosition.z);
				GenerateChildren(nodeRightTop.value(), parentDepth+1);
			}
			if (nodeRightBottom.has_value())
			{
				//GenerateChildren(nodeRightBottom.value(), parentDepth+1);
			}
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
		std::bitset<size> closestTilePos;
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
		auto nextNode = GetNode(closestTilePos, depth);
		BuildTree(playerPos, nextNode,++depth);
	}

}