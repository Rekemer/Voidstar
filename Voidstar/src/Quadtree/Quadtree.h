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
        int depth;
        float tileWidth;
        bool isDrawn = true;

        bool operator==(const Node& node)
        {
            return node.index == index;
        }
    };
    struct Box
    {
        glm::vec2 min{ inf,  inf };
        glm::vec2 max{ -inf, -inf };

        glm::vec2 Middle(glm::vec2 const& p1, glm::vec2 const& p2)
        {
            return { (p1.x + p2.x) / 2.f, (p1.y + p2.y) / 2.f };
        }
        Box& operator |= (glm::vec2 const& p)
        {
            min.x = std::min(min.x, p.x);
            min.y = std::min(min.y, p.y);
            max.x = std::max(max.x, p.x);
            max.y = std::max(max.y, p.y);
            return *this;
        }

    };
    struct GeneratedChildren
    {
        std::bitset<size>  leftTop, rightTop, leftBottom, rightBottom;
    };
    struct Quadtree
    {
        Box bbox;
        Node root;
        Map<int,std::vector<Node>> nodes;

        static Quadtree Build(glm::vec3 posPlayer);
        Node& GetNode(std::bitset<size> node, int depth);
        std::optional<Node> GetLeft(Coordinate coord, std::bitset<size> node, int depthOfNode);
        void Clear(Node& node);
        GeneratedChildren GenerateChildren(Node& node, int depth);
    private:
        void BuildTree(glm::vec3 playerPos, Node nodeToDivide, int depth);
    };
}
