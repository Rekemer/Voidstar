#include"Prereq.h"
#include "Quadtree.h"
#include "Log.h"


namespace Voidstar
{
	//http://www.lcad.icmc.usp.br/~jbatista/procimg/quadtree_neighbours.pdf

	const float groundSize = 10;
	const int widthGround = 2;
	const int heightGround = 2;

	constexpr float levelOfDetail = 12;




	
	std::bitset<size> GenerateTx(int depth)
	{
		std::bitset<size> tx;
		for (int i = 0; i < depth*2; i+=2)
		{
			tx[i] = 0;
			tx[i+1] =1;
		}
		return tx;
	}
	std::bitset<size> GenerateTy(int depth)
	{
		std::bitset<size> ty;
		for (int i = 0; i < depth*2; i+= 2)
		{
			ty[i] = 1;
			ty[i + 1] =0;
		}
		return ty;
	}
	

	bool isRoot(Node& node)
	{
		return node.tileWidth == groundSize;
	}

	// 2 bits per depth
	//  deeper level - go left
	std::bitset<size> ChildOfParent(int x, std::bitset<size> parentIndex, int childrenDepth)
	{
		std::bitset<size> nodeCoords;
		nodeCoords |= x;

		std::bitset<size> maskCoords;
		maskCoords |= std::bitset<size>(parentIndex);
		maskCoords <<=  2;
		nodeCoords |= maskCoords;
		//std::cout << " newNode of "<< x << "coords "<< parentIndex<< " is" << maskCoords << std::endl;
		return nodeCoords;

	}
	std::bitset<size> ParentOfChild(std::bitset<size> child, int childDepth)
	{
		//std::bitset<size> maskCoords;
		//for (int i = 0; i < childDepth * 2 - 3; i++)
		//{
		//	maskCoords[i] = 1;
		//	maskCoords[i + 1] = 1;
		//}
		//std::cout << " mask " << child << std::endl;
		return child>>2;
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
		

		result.nodes[0].emplace_back(result.root);
		//result.GenerateChildren(result.root, 1);
		result.BuildTree(posPlayer, result.root,1);
		return result;
	}
	
	std::optional<Node*> Quadtree::GetNode(std::bitset<size> node, int depth)
	{
		auto& vector = nodes[depth];
		for (int i = 0; i < vector.size(); i++)
		{
			if (vector[i].index == node)
			{
				return &vector[i];
			}
		}
		return{};
	}

	

	//std::bitset<size> west("010101");
	//std::bitset<size> east("000001");
	//std::bitset<size> south("101010");
	//std::bitset<size> north("000010");
	//std::bitset<size> southEast("101011");
	//std::bitset<size> northEast("000011");
	//std::bitset<size> northWest("010111");
	//std::bitset<size> southWest("111111");

	std::bitset<size> GetBitDirection(Direction direction, int depth)
	{
		switch (direction)
		{
		case Voidstar::Direction::NORTH:
			return std::bitset<size>("10");
			break;
		case Voidstar::Direction::EAST:
			return std::bitset<size>("1");
			break;
		case Voidstar::Direction::WEST:
			return GenerateTy(depth);
			break;
		case Voidstar::Direction::SOUTH:
			return GenerateTx(depth);
			break;

		case Voidstar::Direction::SOUTHEAST:
			return GenerateTx(depth) | std::bitset<size>("11");
			break;
		case Voidstar::Direction::SOUTHWEST:
		{
			auto bitDirection = std::bitset<size>();
			bitDirection.set();
			return bitDirection;
		}
			break;

		case Voidstar::Direction::NORTHWEST:
			return GenerateTy(depth) | std::bitset<size>("111");
			break;
		case Voidstar::Direction::NORTHEAST:
			return std::bitset<size>("11");
			break;

		default:
			break;
		}
	}

	std::bitset<size> Increment(std::bitset<size> node,  Direction direction,int depth)
	{
		std::bitset<size> tx = GenerateTx(depth);
		std::bitset<size> ty = GenerateTy(depth);
		auto dir = GetBitDirection(direction,depth);
	
		unsigned long long nodeValue = node.to_ullong();
		unsigned long long dirValue = dir.to_ullong();

		nodeValue = ((((nodeValue | ty.to_ullong()) + (dirValue & tx.to_ullong())) & tx.to_ullong()) | (((nodeValue | tx.to_ullong()) + (dirValue & ty.to_ullong())) & ty.to_ullong()));

		auto neighbour = std::bitset<size>(nodeValue);
		return neighbour;
	}

	//depth 1 = 2
	//depth 2 = 4
	//depth 3 = 5



	std::bitset<size>  Quadtree::GetSibling(Coordinate coord, std::bitset<size> node, int depthOfNode)
	{
		//GetIndexNeighbour(1, node, depthOfNode);
		
		std::bitset<size> sibling;
		if (coord == 1 || coord == 3)
		{
			//sibling = IncrememtX(node, depthOfNode);
		}
		else if (coord == 0 || coord == 2)
		{
		//	sibling = DecrementX(node, depthOfNode);
		}
		
		//node[depthOfNode * 2 - 2] = temp[0];
		//node[depthOfNode*2 -1] = temp[1];
		return sibling;


	}

	bool leftOverflow = false;
	bool rightOverflow = false;
	bool  topOverflow = false;
	bool bottomOverflow = false;
	void Quadtree::Detalize(std::bitset<size> nodeIndex, int depth, int maxDepth, Direction direction)
	{
		if (depth > maxDepth ) return;
		std::bitset<size> iterIndex = nodeIndex;
		//for (int i = depth; i < maxDepth; i++)
		//{
			auto node = GetNode(iterIndex, depth);
			auto children = GenerateChildren(*node.value(), depth+1);
			if (direction == Direction::WEST)
			{
	
				auto a = GetNode(children.rightTop, depth + 1);
				auto b = GetNode(children.rightBottom, depth + 1);
				if (a.has_value())
				{
					//Detalize(a.value()->index, depth + 1, maxDepth, direction);
	
				}
				if (b.has_value())
				{
	
					//Detalize(b.value()->index, depth +1,maxDepth,direction);
				}
			}
		//}
	}

	std::optional<Node> Quadtree::GetNeighbour(Direction direction, std::bitset<size> node, int depthOfNode)
	{


		auto siblingIndex = Increment(node, direction, depthOfNode);
	//	if (depthOfNode == 4)
		{

			//::cout << " current " << node << std::endl;
			//::cout << depthOfNode << " " << siblingIndex << std::endl;
		}

		std::optional<Node*> sibling = GetNode(siblingIndex, depthOfNode);
		auto depthOfNeighbour = depthOfNode;
		while (!sibling.has_value() && depthOfNeighbour > 0)
		{
			auto parent = ParentOfChild(siblingIndex, depthOfNeighbour--);
			siblingIndex = parent;
			sibling = GetNode(parent, depthOfNeighbour);
			if (sibling.has_value() && sibling.value()->isDrawn)
			{
				break;
			}
		}
		if (sibling.has_value() && !isRoot(*sibling.value()) && sibling.value()->isDrawn)
		{
			//Detalize(sibling.value()->index, sibling.value()->depth , depthOfNode,direction);
			return *sibling.value();
		}
		return{};
	
	}
	


	void Quadtree::Clear(Node& node)
	{
	}

	

	GeneratedChildren Quadtree::GenerateChildren(Node& node,int depth)
	{

		
		//auto parentDepth = depth - 1;
		//if (parentDepth != 0)
		//{
		auto node1 = GetNode(node.index, depth - 1);
		node1.value()->isDrawn = false;
		//}
		
		float tileScale = node.tileWidth / 2;
		auto centerOfParentTile = node.worldPosition;

		auto childDepth = depth;
		Node leftTop;
		leftTop.worldPosition.x = centerOfParentTile.x + tileScale / 2;
		leftTop.worldPosition.y = 0.f;
		leftTop.worldPosition.z = centerOfParentTile.z + tileScale / 2;
		leftTop.index = ChildOfParent(2, node.index, childDepth);
		leftTop.tileWidth = tileScale;
		leftTop.depth = depth;
		Node rightTop;

		rightTop.worldPosition.x = centerOfParentTile.x - tileScale / 2;
		rightTop.worldPosition.y = 0.f;
		rightTop.worldPosition.z = centerOfParentTile.z + tileScale / 2;
		rightTop.index = ChildOfParent(3, node.index, childDepth);
		rightTop.tileWidth = tileScale;
		rightTop.depth = depth;
		Node leftBottom;

		leftBottom.worldPosition.x = centerOfParentTile.x + tileScale / 2;
		leftBottom.worldPosition.y = 0.f;
		leftBottom.worldPosition.z = centerOfParentTile.z - tileScale / 2;
		leftBottom.index = ChildOfParent(0, node.index, childDepth);
		leftBottom.tileWidth = tileScale;
		leftBottom.depth = depth;

		Node rightBottom;


		rightBottom.worldPosition.x = centerOfParentTile.x - tileScale / 2;
		rightBottom.worldPosition.y = 0.f;
		rightBottom.worldPosition.z = centerOfParentTile.z - tileScale / 2;
		rightBottom.index = ChildOfParent(1, node.index, childDepth);
		rightBottom.tileWidth = tileScale;
		rightBottom.depth = depth;


		nodes[childDepth].emplace_back(leftTop);
		nodes[childDepth].emplace_back(rightTop);
		nodes[childDepth].emplace_back(leftBottom);
		nodes[childDepth].emplace_back(rightBottom);
		return  { leftTop.index,rightTop.index,leftBottom.index,rightBottom.index };
	

	}


	//void Quadtree::DetaliseNode(std::bitset<size> node, Node& player)
	//{
	//
	//}
	
	// divide node to level depth
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


		if (depth > 1)
		{
			if (nodeToDivide.index == std::bitset<size>("000101"))
			{
			//	std::cout << "sd";
			}
			//std::cout << nodeToDivide.index << std::endl;
			auto parentDepth = depth-1;
			
			auto nodeUp = GetNeighbour(Direction::NORTH, nodeToDivide.index, nodeToDivide.depth);
			auto nodeLeft = GetNeighbour(Direction::WEST, nodeToDivide.index, nodeToDivide.depth);
			
			auto nodeRight = GetNeighbour(Direction::EAST, nodeToDivide.index, nodeToDivide.depth);
			auto nodeBottom = GetNeighbour(Direction::SOUTH, nodeToDivide.index, nodeToDivide.depth);
			
			auto nodeRightBottom = GetNeighbour(Direction::SOUTHEAST, nodeToDivide.index, nodeToDivide.depth);
			auto nodeRightTop = GetNeighbour(Direction::NORTHEAST, nodeToDivide.index, nodeToDivide.depth);
			auto nodeLeftTop = GetNeighbour(Direction::NORTHWEST, nodeToDivide.index, nodeToDivide.depth);
			auto nodeLeftBottom = GetNeighbour(Direction::SOUTHWEST, nodeToDivide.index, nodeToDivide.depth);

			 leftOverflow = ((nodeToDivide.worldPosition.x)) - (groundSize/2 - nodeToDivide.tileWidth / 2) >= 0;
			 rightOverflow = ((nodeToDivide.worldPosition.x)) + (groundSize/2 - nodeToDivide.tileWidth / 2) <= 0;
			 topOverflow = ((nodeToDivide.worldPosition.z)) - (groundSize/2 - nodeToDivide.tileWidth / 2) >= 0;
			 bottomOverflow = ((nodeToDivide.worldPosition.z)) + (groundSize/2 - nodeToDivide.tileWidth / 2) <= 0;

			#define DIAGONAL 1
			#define LEFT 1
			#define RIGHT 1
			#define UP 1
			#define BOTTOM 1

#if  LEFT
			if (nodeLeft.has_value())
			{
				//Log::GetLog()->info("{0} {1} {2}",nodeLeftTop.value().worldPosition.x , nodeLeftTop.value().worldPosition.y, nodeLeftTop.value().worldPosition.z);
				bool isOverflow = leftOverflow;
				if (!isOverflow && nodeLeft.value().isDrawn == true)
				{

					auto children = GenerateChildren(nodeLeft.value(), nodeLeft.value().depth +1);
					auto leftTop = GetNode(children.leftTop, nodeLeft.value().depth + 1);
					auto leftBottom = GetNode(children.leftBottom, nodeLeft.value().depth + 1);
					leftBottom.value()->edges[3] = 0.5f;
					leftTop.value()->edges[3] = 0.5f;

				}
			}
#else
#endif //  LEFT

#if UP

			if (nodeUp.has_value())
			{
				bool isOverflow = topOverflow;

				if (!isOverflow)
				{


					auto children = GenerateChildren(nodeUp.value(), nodeUp.value().depth + 1);;
					auto leftTop = GetNode(children.leftTop, nodeUp.value().depth + 1);
					auto rightTop = GetNode(children.rightTop, nodeUp.value().depth + 1);
					rightTop.value()->edges[2] = 0.5f;
					leftTop.value()->edges[2] = 0.5f;
				}
			}
#else
#endif // UP
#if RIGHT
			if (nodeRight.has_value())
			{
				bool isOverflow = rightOverflow;

				if (!isOverflow && nodeRight.value().isDrawn == true)
				{
					auto children = GenerateChildren(nodeRight.value(), nodeRight.value().depth + 1);;
					auto rightTop = GetNode(children.rightTop, nodeRight.value().depth + 1);
					auto rightBottom = GetNode(children.rightBottom, nodeRight.value().depth + 1);
					rightTop.value()->edges[1] = 0.5f;
					rightBottom.value()->edges[1] = 0.5f;
				}
			}
#else
#endif // RIGHT

#if BOTTOM
			if (nodeBottom.has_value())
			{
				bool isOverflow = bottomOverflow;

				if (!isOverflow && nodeBottom.value().isDrawn == true)
				{

					auto children = GenerateChildren(nodeBottom.value(), nodeBottom.value().depth + 1);
					auto leftBottom = GetNode(children.leftBottom, nodeBottom.value().depth + 1);
					auto rightBottom = GetNode(children.rightBottom, nodeBottom.value().depth + 1);
					leftBottom.value()->edges[0] = 0.5f;
					rightBottom.value()->edges[0] = 0.5f;
				}
			}
#else
#endif // BOTTOM

			#if DIAGONAL
			if (nodeLeftBottom.has_value())
			{
			
				bool isOverflow = bottomOverflow || leftOverflow;
				std::cout << "is overflow " << isOverflow << std::endl;
				//std::cout << "tile world pos x " << nodeToDivide.worldPosition.x << std::endl;
				//std::cout << "boundary x " << (groundSize / 2 - nodeToDivide.tileWidth / 2) << std::endl;
				//std::cout << "distance x " << (glm::abs(nodeToDivide.worldPosition.x)) - (groundSize / 2 - nodeToDivide.tileWidth / /2)/ << std::endl;
				if (!isOverflow && nodeLeftBottom.value().isDrawn==true)
				{
					auto children = GenerateChildren(nodeLeftBottom.value(), nodeLeftBottom.value().depth + 1);
					auto leftTop = GetNode(children.leftTop, nodeLeftBottom.value().depth + 1);
					auto leftBottom = GetNode(children.leftBottom, nodeLeftBottom.value().depth + 1);
					auto rightBottom = GetNode(children.rightBottom, nodeLeftBottom.value().depth + 1);
			
					leftTop.value()->edges[3] = 0.5f;
					leftBottom.value()->edges[3] = 0.5f;
					leftBottom.value()->edges[0] = 0.5f;
					rightBottom.value()->edges[0] = 0.5f;
				}
			}
			if (nodeLeftTop.has_value())
			{
				bool isOverflow = topOverflow || leftOverflow;
				//std::cout << "is overflow " << isOverflow << std::endl;
				//std::cout << "tile world pos x " << nodeToDivide.worldPosition.x << std::endl;
				//std::cout << "boundary x " << (groundSize / 2 - nodeToDivide.tileWidth / 2) << std::endl;
				//std::cout << "distance x " << (glm::abs(nodeToDivide.worldPosition.x)) - (groundSize / 2 //nodeToDivide.tileWidth /2) /<< std::endl;
				if (!isOverflow && nodeLeftTop.value().isDrawn == true)
				{
					auto children = GenerateChildren(nodeLeftTop.value(), nodeLeftTop.value().depth + 1);
					auto leftTop = GetNode(children.leftTop, nodeLeftTop.value().depth + 1);
					auto leftBottom = GetNode(children.leftBottom, nodeLeftTop.value().depth + 1);
					auto rightTop = GetNode(children.rightTop, nodeLeftTop.value().depth + 1);
			
					leftTop.value()->edges[3] = 0.5f;
					leftTop.value()->edges[2] = 0.5f;
					leftBottom.value()->edges[3] = 0.5f;
					rightTop.value()->edges[2] = 0.5f;
				}
			}
			if (nodeRightTop.has_value())
			{
				bool isOverflow = topOverflow || rightOverflow;
				//std::cout << "is overflow " << isOverflow << std::endl;
				//std::cout << "tile world pos x " << nodeToDivide.worldPosition.x << std::endl;
				//std::cout << "boundary x " << (groundSize / 2 - nodeToDivide.tileWidth / 2) << std::endl;
				//std::cout << "distance x " << (glm::abs(nodeToDivide.worldPosition.x)) - (groundSize / 2 nodeToDivide.tileWidth / 2) << std::endl;
				if (!isOverflow && nodeRightTop.value().isDrawn == true)
				{
					auto children = GenerateChildren(nodeRightTop.value(), nodeRightTop.value().depth + 1);
					auto leftTop = GetNode(children.leftTop, nodeRightTop.value().depth + 1);
					auto rightTop = GetNode(children.rightTop, nodeRightTop.value().depth + 1);
					auto rightBottom = GetNode(children.rightBottom, nodeRightTop.value().depth + 1);

					leftTop.value()->edges[2] = 0.5f;
					rightTop.value()->edges[2] = 0.5f;
					rightTop.value()->edges[1] = 0.5f;
					rightBottom.value()->edges[1] = 0.5f;
				}
			}
			
			if (nodeRightBottom.has_value())
			{
				bool isOverflow = bottomOverflow || rightOverflow;
				//std::cout << "is overflow " << isOverflow << std::endl;
				//std::cout << "tile world pos x " << nodeToDivide.worldPosition.x << std::endl;
				//std::cout << "boundary x " << (groundSize / 2 - nodeToDivide.tileWidth / 2) << std::endl;
				//std::cout << "distance x " << (glm::abs(nodeToDivide.worldPosition.x)) - (groundSize / 2 //nodeToDivide.tileWidth / 2) << std::endl;
				if (!isOverflow && nodeRightBottom.value().isDrawn == true)
				{
					auto children = GenerateChildren(nodeRightBottom.value(), nodeRightBottom.value().depth + 1);
					auto rightBottom = GetNode(children.rightBottom, nodeRightBottom.value().depth + 1);
					auto rightTop = GetNode(children.rightTop, nodeRightBottom.value().depth + 1);
					auto leftBottom = GetNode(children.leftBottom, nodeRightBottom.value().depth + 1);
					leftBottom.value()->edges[0] = 0.5f;
					rightBottom.value()->edges[1] = 0.5f;
					rightBottom.value()->edges[0] = 0.5f;
					rightTop.value()->edges[1] = 0.5f;
				}
			}
			#else

			#endif
			
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
		BuildTree(playerPos, *nextNode.value(), ++depth);
	}

}