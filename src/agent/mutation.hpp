#pragma once

#include "genome.hpp"
#include "innovationTracker.hpp"
#include "neatUtils.hpp"
#include "../neatConfig.hpp"

#include <random>
#include <algorithm>
#include <vector>
#include <unordered_map>

namespace neat
{
    // Mutate a single random weight (called multiple times per generation)
    int mutateSingleWeight(Genome &genome, std::mt19937 &rng)
    {
        if (genome.connections.empty())
            return 0;
            
        std::uniform_real_distribution<float> uni01(0.f, 1.f);
        std::uniform_real_distribution<float> wdist(mutConf::weightMin, mutConf::weightMax);
        std::normal_distribution<float> largePerturbation(0.f, mutConf::perturbStd);
        std::normal_distribution<float> smallPerturbation(0.f, mutConf::perturbSmall);
        
        // Pick a random enabled connection
        std::vector<std::size_t> enabledIdx;
        for (std::size_t i = 0; i < genome.connections.size(); i++)
        {
            if (genome.connections[i].enabled || mutConf::mutateDisabled)
                enabledIdx.push_back(i);
        }
        
        if (enabledIdx.empty())
            return 0;
            
        std::uniform_int_distribution<std::size_t> pick(0, enabledIdx.size() - 1);
        Connection &c = genome.connections[enabledIdx[pick(rng)]];
        
        float r = uni01(rng);
        
        if (r < mutConf::pReset)
        {
            // 10% - Complete reset to random value
            c.weight = wdist(rng);
        }
        else if (r < mutConf::pReset + 0.20f)
        {
            // 20% - Large perturbation
            c.weight += largePerturbation(rng);
        }
        else
        {
            // 70% - Small perturbation for fine-tuning
            c.weight += smallPerturbation(rng);
        }
        
        c.weight = std::clamp(c.weight, mutConf::weightMin, mutConf::weightMax);
        return 1;
    }

    // Original function for compatibility - mutates all weights
    int mutateWeights(Genome &genome, std::mt19937 &rng)
    {
        std::uniform_real_distribution<float> uni01(0.f, 1.f);
        std::uniform_real_distribution<float> wdist(mutConf::weightMin, mutConf::weightMax);
        std::normal_distribution<float> largePerturbation(0.f, mutConf::perturbStd);
        std::normal_distribution<float> smallPerturbation(0.f, mutConf::perturbSmall);

        int mutated = 0;

        for (auto &c : genome.connections)
        {
            if(!mutConf::mutateDisabled && !c.enabled)
                continue;

            float r = uni01(rng);

            if (r < mutConf::pReset)
            {
                // Complete reset
                c.weight = wdist(rng);
                mutated++;
            }
            else if (r < mutConf::pReset + mutConf::pPerturb)
            {
                // Perturbation - mix of large and small
                if (uni01(rng) < 0.25f)
                    c.weight += largePerturbation(rng);  // 25% large
                else
                    c.weight += smallPerturbation(rng); // 75% small
                    
                c.weight = std::clamp(c.weight, mutConf::weightMin, mutConf::weightMax);
                mutated++;
            }
        }

        return mutated;
    }

    bool mutateAddConnection(
        Genome &genome,
        InnovationTracker &tracker,
        std::mt19937 &rng
    )
    {
        if (genome.nodes.size() < 2)
            return false;

        std::uniform_int_distribution<std::size_t> pick(0, genome.nodes.size() - 1);
        std::uniform_real_distribution<float> wdist(mutConf::weightMin, mutConf::weightMax);

        bool enabledOnly = mutConf::cycleCheckEnabledOnly;

        for (int attempt = 0; attempt < mutConf::addConnMaxTries; attempt++)
        {

            Node &a = genome.nodes[pick(rng)];
            Node &b = genome.nodes[pick(rng)];

            if (a.id == b.id)
                continue;

            auto tryAdd = [&](Node &inN, Node &outN) -> bool
            {
                if (!neat::utils::isValidDirection(inN, outN))
                    return false;
                if (neat::utils::connectionExists(genome, inN.id, outN.id))
                    return false;
                if (neat::utils::wouldCreateCycle(genome, inN.id, outN.id, enabledOnly))
                    return false;
                
                Connection c;
                c.inNode = inN.id;
                c.outNode = outN.id;
                c.weight = wdist(rng);
                c.enabled = true;
                c.innovation = tracker.getInnovation(c.inNode, c.outNode);

                genome.connections.push_back(c);
                
                return true;
            };

            if (tryAdd(a, b) || tryAdd(b, a))
            {
                utils::sortConnectionsByInnovation(genome);
                return true;
            }
        }

        return false;
    }

    bool mutateAddNode(
        Genome &genome,
        InnovationTracker &tracker,
        std::mt19937 &rng
    )
    {
        std::vector<std::size_t> enabledIdx;
        enabledIdx.reserve(genome.connections.size());

        for (std::size_t i = 0; i < genome.connections.size(); i++)
        {
            if (genome.connections[i].enabled)
                enabledIdx.push_back(i);
        }

        if (enabledIdx.empty())
            return false;

        std::uniform_int_distribution<std::size_t> pickConn(0, enabledIdx.size() - 1);
        std::size_t ci = enabledIdx[pickConn(rng)];

        Connection &old = genome.connections[ci];

        // Disable old connection
        old.enabled = false;

        // New hidden node
        int maxId = -1;
        for (auto &n : genome.nodes)
            maxId = std::max(maxId, n.id);

        Node newNode;
        newNode.id = maxId + 1;
        newNode.type = NodeType::Hidden;
        newNode.activation = Activation::Tanh;

        genome.nodes.push_back(newNode);

        // Add two new connections
        // in -> newNode (weight = 1)
        // newNode -> out (weight = oldWeight)

        Connection c1;
        c1.inNode = old.inNode;
        c1.outNode = newNode.id;
        c1.weight = 1.f;
        c1.enabled = true;
        c1.innovation = tracker.getInnovation(c1.inNode, c1.outNode);

        Connection c2;
        c2.inNode = newNode.id;
        c2.outNode = old.outNode;
        c2.weight = old.weight;
        c2.enabled = true;
        c2.innovation = tracker.getInnovation(c2.inNode, c2.outNode);

        genome.connections.push_back(c1);
        genome.connections.push_back(c2);

        utils::sortConnectionsByInnovation(genome);
        return true;
    }

    Genome crossover(
        Genome &parent1,
        Genome &parent2,
        std::mt19937 &rng
    )
    {
        Genome child;

        std::unordered_map<int, Node> n1;
        n1.reserve(parent1.nodes.size());
        
        for (auto &n : parent1.nodes)
            n1[n.id] = n;


        std::unordered_map<int, Node> n2;
        n2.reserve(parent2.nodes.size());

        for (auto &n : parent2.nodes)
            n2[n.id] = n;

        auto ensureNode = [&](int id)
        {
            // Check if in child
            for (auto &n : child.nodes)
                if (n.id == id)
                    return;

            auto it1 = n1.find(id);
            if (it1 != n1.end())
            {
                child.nodes.push_back(it1->second);
                return;
            }

            auto it2 = n2.find(id);
            if (it2 != n2.end())
            {
                child.nodes.push_back(it2->second);
                return;
            }

            Node fallback;
            fallback.id = id;
            fallback.type = NodeType::Hidden;
            fallback.activation = Activation::Tanh;
            child.nodes.push_back(fallback);
        };

        std::unordered_map<int, Connection> c2ByInnov;
        c2ByInnov.reserve(parent2.connections.size());

        for (auto &c : parent2.connections)
            c2ByInnov[c.innovation] = c;

        std::uniform_real_distribution<float> uni01(0.f, 1.f);
        std::uniform_int_distribution<int> coin(0, 1);

        child.connections.reserve(parent1.connections.size());

        for (auto &c1 : parent1.connections)
        {
            auto it2 = c2ByInnov.find(c1.innovation);

            if (it2 == c2ByInnov.end())
            {
                // inherit from fitter parent p1
                child.connections.push_back(c1);
            }

            else
            {
                Connection &c2 = it2->second;
                Connection chosen = (coin(rng) == 0) ? c1 : c2;

                if (!c1.enabled || !c2.enabled)
                {
                    if (uni01(rng) < mutConf::kDisableInheritProb)
                        chosen.enabled = false;
                }

                // innovation mapping same nodes, if not parent1 topology
                if (chosen.inNode != c1.inNode || chosen.outNode != c1.outNode)
                {
                    chosen.inNode = c1.inNode;
                    chosen.outNode = c1.outNode;
                }

                child.connections.push_back(chosen);
            }
        }

        for (auto &c : child.connections)
        {
            ensureNode(c.inNode);
            ensureNode(c.outNode);
        }
        
        utils::sortConnectionsByInnovation(child);
        return child;
    }
}