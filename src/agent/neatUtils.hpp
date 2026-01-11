#pragma once

#include "genome.hpp"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <cmath>

namespace neat
{
    namespace utils
    {
        bool connectionExists(Genome &g, int inId, int outId)
        {
            for (auto &c : g.connections)
            {
                if (c.inNode == inId && c.outNode == outId)
                    return true;
            }
            return false;
        }

        bool wouldCreateCycle(Genome &g, int inId, int outId, bool enabledOnly)
        {
            if (inId == outId)
                return true;
            std::unordered_map<int, std::vector<int>> adj;
            adj.reserve(g.nodes.size());

            for (auto &c : g.connections)
            {
                if (!c.enabled && enabledOnly)
                    continue;
                adj[c.inNode].push_back(c.outNode);
            }

            std::queue<int> q;
            std::unordered_set<int> vis;
            vis.reserve(g.nodes.size());

            q.push(outId);
            vis.insert(outId);

            while (!q.empty())
            {
                int u = q.front();
                q.pop();

                if (u == inId)
                    return true;

                auto it = adj.find(u);

                if (it == adj.end())
                    continue;
                
                for (int v : it->second)
                {
                    if (vis.insert(v).second)
                        q.push(v);
                }
            }
            return false;
        }

        bool isValidDirection(Node &inN, Node &outN)
        {
            if (outN.type == NodeType::Input || outN.type == NodeType::Bias)
                return false;
            
            if (inN.type == NodeType::Output)
                return false;

            return true;
        }

        inline void sortConnectionsByInnovation(Genome& g)
        {
            std::sort(g.connections.begin(), g.connections.end(),
                [](const Connection& a, const Connection& b)
                {
                    return a.innovation < b.innovation;
                });
        }

    }   
}