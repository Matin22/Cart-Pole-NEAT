#pragma once

#include "genome.hpp"

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstddef>
#include <queue>
#include <cmath>

namespace neat
{
    float activate(Activation a, float x)
    {
        switch (a)
        {
        case Activation::Linear:
            return x;
        case Activation::Tanh:
            return std::tanh(x);
        case Activation::Sigmoid:
            return 1.f / (1.f + std::exp(-x));
        case Activation::Relu:
            return (x > 0.f) ? x : 0.f;
        default:
            return std::tanh(x);
        }
    }

    std::vector<float> forwardPass(Genome& g, std::vector<float>& inputValues)
    {
        // Map node -> g.nodes index
        std::unordered_map<int, std::size_t> idToIndex;
        idToIndex.reserve(g.nodes.size());
        for (std::size_t i = 0; i < g.nodes.size(); i++)
            idToIndex[g.nodes[i].id] = i;
        
        // collect inout node ids
        std::vector<int> inputNodeIds;
        inputNodeIds.reserve(g.nodes.size());
        for (auto& n : g.nodes)
            if (n.type == NodeType::Input)
                inputNodeIds.push_back(n.id);

        std::sort(inputNodeIds.begin(), inputNodeIds.end());

        struct inEdge {
            int in;
            float w;
        };

        std::unordered_map<int, std::vector<inEdge>> incoming;
        incoming.reserve(g.nodes.size());

        std::unordered_map<int, int> inDegree;
        inDegree.reserve(g.nodes.size());
        for (auto &n : g.nodes)
        {
            inDegree[n.id] = 0;
        }

        for (auto &c : g.connections)
        {
            if (!c.enabled) continue;
            
            if (!idToIndex.contains(c.inNode) || !idToIndex.contains(c.outNode)) continue;

            incoming[c.outNode].push_back({c.inNode, c.weight});
            inDegree[c.outNode] += 1;
        }

        // Kahns algo
        std::queue<int> q;
        for (auto &n : g.nodes)
        {
            if (inDegree[n.id] == 0)
            {
                q.push(n.id);
            }
        }

        std::unordered_map<int, float> value;
        value.reserve(g.nodes.size());

        // init input
        for (std::size_t i = 0; i < inputNodeIds.size() && i < inputValues.size(); i++)
        {
            value[inputNodeIds[i]] = inputValues[i];
        }

        for (auto &n : g.nodes)
        {
            if (n.type == NodeType::Bias)
                value[n.id] = 1.f;
        }

        std::size_t processed = 0;

        while (!q.empty())
        {
            int nodeId = q.front();
            q.pop();
            processed++;

            auto &node = g.nodes[idToIndex[nodeId]];

            if (node.type != NodeType::Input && node.type != NodeType::Bias)
            {
                float sum = 0.f;
                auto it = incoming.find(nodeId);
                if (it != incoming.end())
                {
                    for (auto &e : it->second)
                        sum += value[e.in] * e.w;
                }

                value[nodeId] = activate(node.activation, sum);
            }

            else
            {
                if (!value.contains(nodeId))
                    value[nodeId] = 0.f;
            }

            for (auto &c : g.connections)
            {
                if (!c.enabled) continue;
                if (c.inNode != nodeId) continue;
                if (!inDegree.contains(c.outNode)) continue;

                inDegree[c.outNode] -= 1;
                if (inDegree[c.outNode] == 0)
                    q.push(c.outNode);
            }
        }

        (void)processed;

        std::vector<int> outNodeIds;
        for (auto &n : g.nodes)
            if(n.type == NodeType::Output)
                outNodeIds.push_back(n.id);

        std::sort(outNodeIds.begin(), outNodeIds.end());

        std::vector<float> outputs;
        outputs.reserve(outNodeIds.size());
        for (int oid : outNodeIds)
            outputs.push_back(value.contains(oid) ? value[oid] : 0.f);

        return outputs;
    }
}