#pragma once
#include"../Prereq.h"
#include"glm.hpp"
namespace Voidstar
{
    using NodeID = uint32_t;
    static constexpr float inf = std::numeric_limits<float>::infinity();
    static constexpr NodeID null = NodeID(-1);

    struct Node
    {
        NodeID children[2][2]{
            {null, null},
            {null, null}
        };
        NodeID parentId;
        glm::vec3 worldPosition;
        float tileWidth;
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
    struct Quadtree
    {
        Box bbox;
        NodeID root;
        std::vector<Node> nodes;

        static Quadtree Build(glm::vec2& posPlayer);
        Node& GetLeft(Node& currentNode);
        Node& GetRight(Node& currentNode);
        Node& GetUp(Node& currentNode);
        Node& GetBottom(Node& currentNode);
        void Clear(Node& node);
        void GenerateChildren(Node& node);
    private:
        void BuildTree();
    };
}
