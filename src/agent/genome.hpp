#pragma once

#include <vector>

namespace neat
{
    enum class NodeType
    {
        Input = 0,
        Hidden = 1,
        Output = 2,
        Bias = 3
    };

    enum class Activation
    {
        Linear = 0,
        Tanh = 1,
        Sigmoid = 2,
        Relu = 3
    };

    struct Node
    {
        int id = -1;
        NodeType type = NodeType::Hidden;
        Activation activation = Activation::Tanh;
    };

    struct Connection
    {
        int inNode = -1;
        int outNode = -1;

        float weight = 0.f;
        bool enabled = true;

        int innovation = -1;
    };

    struct Genome
    {
        std::vector<Node> nodes;
        std::vector<Connection> connections;

        float fitness = 0.f;
    };
    
}