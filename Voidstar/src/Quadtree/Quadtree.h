#pragma once
#include"../Prereq.h"
#include"glm.hpp"
#include <unordered_map>
#include <bitset>
#include <optional>
namespace Voidstar
{
    constexpr size_t size = 64;
    #define F_32 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    #define F_16 0xFFFFFFFFFFFFFFFF
    template<typename U ,typename T>
    using Map = std::unordered_map<U,T>;
    using Coordinate = uint32_t;
    static constexpr float inf = std::numeric_limits<float>::infinity();
    static constexpr Coordinate null = Coordinate(-1);
    struct Node
    {
        std::bitset<size> index;
        glm::vec3 worldPosition;
        float edges[4] = {1,1,1,1};
        int depth;
        float tileWidth;
        bool isDrawn = true;

        bool operator==(const Node& node)
        {
            return node.index == index;
        }
    };

    enum class Direction
    {
        NORTH,
        EAST,
        WEST,
        SOUTH,

        SOUTHEAST,
        SOUTHWEST,

        NORTHWEST,
        NORTHEAST,
    };   

 
    struct GeneratedChildren
    {
        std::bitset<size>  leftTop, rightTop, leftBottom, rightBottom;
    };
    class Quadtree
    {
    public:
        Node root;
        Map<int,std::vector<Node>> nodes;

        static Quadtree Build(glm::vec3 posPlayer);
        std::optional<Node*> GetNode(std::bitset<size> node, int depth);
        std::optional<Node> GetNeighbour(Direction direction, std::bitset<size> node, int depthOfNode);
        void Clear(Node& node);
        GeneratedChildren GenerateChildren(Node& node, int depth);
        ~Quadtree();
    private:
        void BuildTree(glm::vec3 playerPos, Node nodeToDivide, int depth);
    };
}
