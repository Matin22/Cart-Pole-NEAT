#pragma once

#include "genome.hpp"
#include "innovationTracker.hpp"

#include <random>

namespace neat
{
    Genome CreateMinimalGenome(
        int nInputs,
        int nOutputs,
        InnovationTracker& tracker,
        std::mt19937 &rng
    )
    {
        Genome g;

        if (nInputs <= 0 ||nOutputs <= 0)
            return g;

        g.nodes.reserve(static_cast<std::size_t>(nInputs + nOutputs));

        // --- Create inputs ---
        for (int i = 0; i < nInputs; i++)
        {
            Node n;
            n.id = i;
            n.type = NodeType::Input;
            n.activation = Activation::Linear;
            g.nodes.push_back(n);
        }

        // --- Create outputs ---
        for (int j = 0; j < nOutputs; j++)
        {
            Node n;
            n.id = nInputs + j;
            n.type = NodeType::Output;
            n.activation = Activation::Tanh;
            g.nodes.push_back(n);
        }

        g.connections.reserve(static_cast<std::size_t>(nInputs) * static_cast<std::size_t>(nOutputs));

        std::uniform_real_distribution<float> wdist(-1.0f, 1.0f);

        for (int in = 0; in < nInputs; in++)
        {
            for (int out = 0; out < nOutputs; out++)
            {
                Connection c;
                c.inNode = in;
                c.outNode = nInputs + out;
                c.weight = wdist(rng);
                c.enabled = true;
                c.innovation = tracker.getInnovation(c.inNode, c.outNode);

                g.connections.push_back(c);
            }
        }

        return g;
    }
}