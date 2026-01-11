#pragma once

#include "genome.hpp"
#include "mutation.hpp"
#include "speciation.hpp"
#include "fitness.hpp"
#include "../neatConfig.hpp"

#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <cstddef>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace neat
{
    struct GenerationStats
    {
        float bestFitness = 0.f;
        float meanFitness = 0.f;
        std::size_t speciesCount = 0;
    };

    std::size_t tournamentPick(
        std::vector<std::size_t> &pool,
        std::vector<float> &fitness,
        std::mt19937 &rng,
        int k
    )
    {
        if (pool.empty())
            return 0;  // Safety check
        
        if (pool.size() == 1)
            return pool[0];  // Only one option
            
        std::uniform_int_distribution<std::size_t> pick(0, pool.size() - 1);

        std::size_t best = pool[pick(rng)];
        for (int i = 1; i < k && i < static_cast<int>(pool.size()); i++)
        {
            std::size_t cand = pool[pick(rng)];
            if (fitness[cand] > fitness[best])
                best = cand;
        }

        return best;
    }

    std::vector<Genome> evolvePopulation(
        std::vector<Genome> &population,
        InnovationTracker &tracker,
        std::mt19937 &rng,
        float(*evaluateFitness) (Genome&, std::mt19937&),
        GenerationStats *outStats = nullptr,
        Genome* outBestGenome = nullptr
    )
    {
        std::size_t popN = static_cast<std::size_t>(evoConf::populationSize);
        if (population.empty())
            return {};

        // --- Evaluate (parallel) ---
        std::vector<float> fitness(population.size(), 0.f);

        #ifdef _OPENMP
        #pragma omp parallel
        {
            // Each thread gets its own RNG seeded differently
            std::mt19937 threadRng(rng() + omp_get_thread_num());
            
            #pragma omp for schedule(dynamic)
            for (std::size_t i = 0; i < population.size(); i++)
            {
                float f = evaluateFitness(population[i], threadRng);
                population[i].fitness = f;
                fitness[i] = f;
            }
        }
        #else
        // Single-threaded fallback
        for (std::size_t i = 0; i < population.size(); i++)
        {
            float f = evaluateFitness(population[i], rng);
            population[i].fitness = f;
            fitness[i] = f;
        }
        #endif

        // Compute stats after parallel section
        float sumFitness = 0.f;
        float minFit = 1e30f;
        std::size_t bestIdx = 0;
        float bestFit = -1e30f;

        for (std::size_t i = 0; i < population.size(); i++)
        {
            float f = fitness[i];
            sumFitness += f;
            if (f < minFit) minFit = f;
            if (f > bestFit) { bestFit = f; bestIdx = i; }
        }

        float shift = (minFit < 0.f) ? (-minFit + 1e-3f) : 0.f;

        // --- Speciate ---
        std::vector<Species> species = speciatePopulation(population);
        if (species.empty())
            return population;
        
        // --- Compute adjusted fitness ---
        std::vector<float> speciesScore(species.size(), 0.f);
        float totalScore = 0.f;

        for (std::size_t si = 0; si < species.size(); si++)
        {
            auto &members = species[si].members;
            if (members.empty())
                continue;
            float denom = evoConf::useFitnessSharing ? static_cast<float>(members.size()) : 1.f;

            float sum = 0.f;
            for (auto idx : members)
            {
                float fAdj = fitness[idx] + shift;
                sum += fAdj / denom;
            }

            speciesScore[si] = std::max(0.f, sum);
            totalScore += speciesScore[si];
        }

        if (totalScore <= 0.f)
        {
            // equal if all scores are zero or negative
            totalScore = static_cast<float>(species.size());
            for (auto &s : speciesScore)
                s = 1.f;
        }

        // --- offspring alloc ---
        std::vector<int> offspringCount(species.size(), 0);
        std::vector<float> frac(species.size(), 0.f);

        int allocated = 0;
        for (std::size_t si = 0; si < species.size(); si++)
        {
            float exact = (speciesScore[si] / totalScore) * static_cast<float>(popN);
            int base = static_cast<int>(exact);
            offspringCount[si] = base;
            frac[si] = exact - static_cast<float>(base);
            allocated += base;
        }

        // Distribute remainder by largest fractional parts
        int remaining = static_cast<int>(popN) - allocated;
        std::vector<std::size_t> order(species.size());
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(),
            [&](std::size_t a, std::size_t b)
            { return frac[a] > frac[b]; });

        if (!order.empty())
        {
            for (int r = 0; r < remaining; r++)
                offspringCount[order[r % order.size()]]++;
        }
        
        // Species should keep elites (but cap at actual member count)
        for (std::size_t si = 0; si < species.size(); si++)
        {
            int maxElites = std::min(evoConf::elitesPerSpecies, static_cast<int>(species[si].members.size()));
            offspringCount[si] = std::max(offspringCount[si], maxElites);
        }

        auto totaloff = [&]()
        {
            int t = 0;
            for (auto x : offspringCount)
                t += x;
            return t;
        };

        while (totaloff() > static_cast<int>(popN))
        {
            auto it = std::max_element(offspringCount.begin(), offspringCount.end());
            if (*it > evoConf::elitesPerSpecies)
                (*it)--;
            else
                break;
        }

        std::vector<Genome> next;
        next.reserve(popN);

        std::uniform_real_distribution<float> uni01(0.f, 1.f);

        for (std::size_t si = 0; si < species.size(); ++si)
        {
            auto& members = species[si].members;
            if (members.empty())
                continue;

            // Sort members by fitness descending (for elites)
            std::sort(members.begin(), members.end(),
                      [&](std::size_t a, std::size_t b){ return fitness[a] > fitness[b]; });

            int nOff = offspringCount[si];

            // copy best genomes unchanged (but don't exceed member count)
            int nElite = std::min({evoConf::elitesPerSpecies, nOff, static_cast<int>(members.size())});
            for (int e = 0; e < nElite && next.size() < popN; ++e)
                next.push_back(population[members[e]]);

            // Fill remaining offspring
            for (int k = nElite; k < nOff && next.size() < popN; ++k)
            {
                if (members.empty())
                    break;
                    
                std::size_t pAidx = tournamentPick(members, fitness, rng, evoConf::tournamentK);

                Genome child = population[pAidx];

                // Crossover
                if (uni01(rng) < evoConf::pCrossover && members.size() >= 2)
                {
                    std::size_t pBidx = tournamentPick(members, fitness, rng, evoConf::tournamentK);

                    // Ensure parent1 is fitter for crossover() contract
                    Genome& A = population[pAidx];
                    Genome& B = population[pBidx];
                    if (fitness[pAidx] >= fitness[pBidx])
                        child = crossover(A, B, rng);
                    else
                        child = crossover(B, A, rng);
                }

                // Mutations
                if (uni01(rng) < evoConf::pMutateWeights)  mutateWeights(child, rng);
                if (uni01(rng) < evoConf::pAddConnection)  mutateAddConnection(child, tracker, rng);
                if (uni01(rng) < evoConf::pAddNode)        mutateAddNode(child, tracker, rng);

                next.push_back(std::move(child));
            }
        }

        // If rounding/elitism caused shortfall, fill by cloning best overall
        if (next.size() < popN)
        {
            std::size_t best = 0;
            for (std::size_t i = 1; i < population.size(); ++i)
                if (fitness[i] > fitness[best]) best = i;

            while (next.size() < popN)
                next.push_back(population[best]);
        }

        // Trim if overshot
        if (next.size() > popN)
            next.resize(popN);

        if (outStats)
        {
            outStats->bestFitness = bestFit;
            outStats->meanFitness = sumFitness / static_cast<float>(population.size());
            outStats->speciesCount = species.size();
        }

        if (outBestGenome)
            *outBestGenome = population[bestIdx];

        return next;
    }
}
