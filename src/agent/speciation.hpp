#pragma once

#include "genome.hpp"
#include "../neatConfig.hpp"

#include <vector>
#include <algorithm>
#include <cmath>
#include <cstddef>

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
        std::vector<Genome> &population
    )
    {
        std::vector<Species> species;
        species.reserve(population.size());

        for (std::size_t i = 0; i < population.size(); i++)
        {
            bool assigned = false;

            for (auto &s : species)
            {
                float d = compatibilityDistance(population[i], population[s.representative]);
                if (d < specConf::threshold)
                {
                    s.members.push_back(i);
                    assigned = true;
                    break;
                }
            }

            if (!assigned)
            {
                Species s;
                s.representative = i;
                s.members.push_back(i);
                species.push_back(std::move(s));
            }
        }

        return species;
    }
}