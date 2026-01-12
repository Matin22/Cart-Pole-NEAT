#pragma once

#include "genome.hpp"
#include "../neatConfig.hpp"

#include <vector>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <random>
#include <numeric>

namespace neat
{
    struct Species
    {
        std::size_t representative;
        std::vector<std::size_t> members;
    };

    float compatibilityDistance(
        Genome &g1,
        Genome &g2
    )
    {
        std::vector<Connection> &a = g1.connections;
        std::vector<Connection> &b = g2.connections;

        auto byInnov = [](Connection &x, Connection &y)
        {
            return x.innovation < y.innovation;
        };

        std::sort(a.begin(), a.end(), byInnov);
        std::sort(b.begin(), b.end(), byInnov);

        int nA = static_cast<int>(a.size());
        int nB = static_cast<int>(b.size());
        int nRaw = std::max(nA, nB);
        float N = (nRaw < specConf::smallGenomeCutoff) ? 1.f : static_cast<float>(nRaw);

        int E = 0;              // excess
        int D = 0;              // disjoint
        int M = 0;              // matching
        float wDiffSum = 0.f;   // sum om |w1 - w2| matching

        int i = 0, j = 0;

        while (i < nA && j < nB)
        {
            int ia = a[i].innovation;
            int ib = b[j].innovation;

            if (ia == ib)
            {
                M++;
                wDiffSum += std::fabs(a[i].weight - b[j].weight);
                i++;
                j++;
            }
            
            else if (ia < ib)
            {
                D++;
                i++;
            }

            else if (ia > ib)
            {
                D++;
                j++;
            }
        }

        if (i < nA)
            E += (nA - i);

        if (j < nB)
            E += (nB - j);

        float W = (M > 0) ? (wDiffSum / static_cast<float>(M)) : 0.f;

        return (specConf::c1 * static_cast<float>(E)) / N
            + (specConf::c2 * static_cast<float>(D)) / N
            + (specConf::c3 * W);
    }

    std::vector<Species> speciatePopulation(
        std::vector<Genome> &population,
        float thresholdOverride = -1.0f
    )
    {
        std::vector<Species> species;
        species.reserve(population.size());

        float thresh = (thresholdOverride > 0.0f) ? thresholdOverride : specConf::threshold;

        // Shuffle order to avoid always choosing the same representatives
        std::vector<std::size_t> order(population.size());
        std::iota(order.begin(), order.end(), 0);
        // Use a deterministic shuffle to avoid potential random_device issues
        std::mt19937 rng(1234567u);
        std::shuffle(order.begin(), order.end(), rng);

        for (std::size_t idx : order)
        {
            bool assigned = false;

            for (auto &s : species)
            {
                float d = compatibilityDistance(population[idx], population[s.representative]);
                if (d < thresh)
                {
                    s.members.push_back(idx);
                    assigned = true;
                    break;
                }
            }

            if (!assigned)
            {
                Species s;
                s.representative = idx;
                s.members.push_back(idx);
                species.push_back(std::move(s));
            }
        }

        // Fallback: ensure at least two species to maintain diversity
        if (species.size() < 2 && population.size() > 1)
        {
            // Only split if first species has at least two members
            if (!species.empty() && species[0].members.size() > 1)
            {
                Species second;
                second.representative = order.size() > 1 ? order[1] : order[0];
                // Move roughly half of members from the first species to the second
                auto &firstMembers = species[0].members;
                for (std::size_t i = 1; i < firstMembers.size(); i += 2)
                {
                    second.members.push_back(firstMembers[i]);
                }
                // Remove moved members from the first species
                std::vector<std::size_t> keep;
                keep.reserve(firstMembers.size());
                for (std::size_t i = 0; i < firstMembers.size(); ++i)
                {
                    if (i % 2 == 0) keep.push_back(firstMembers[i]);
                }
                firstMembers.swap(keep);
                species.push_back(std::move(second));
            }
        }

        return species;
    }
}