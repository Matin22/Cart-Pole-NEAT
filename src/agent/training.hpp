#pragma once

#include "genome.hpp"
#include "createMinimalGenome.hpp"
#include "innovationTracker.hpp"
#include "forwardpass.hpp"
#include "evolution.hpp"
#include "fitness.hpp"
#include "../neatConfig.hpp"
#include "../environment/cart.hpp"
#include "../environment/pendulum.hpp"

#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <string>
#include <iostream>
#include <cmath>

namespace neat
{
    struct TrainingReport
    {
        Genome bestGenome;
        float bestFitness = -1e30f;
        int generationsRun = 0;
    };

    std::vector<Genome> initPop(
        InnovationTracker &tracker,
        std::mt19937 &rng
    )
    {
        std::vector<Genome> pop;
        pop.reserve(static_cast<size_t>(evoConf::populationSize));

        for (int i = 0; i < evoConf::populationSize; i++)
        {
            pop.push_back(CreateMinimalGenome(5, 1, tracker, rng));
        }
        return pop;
    }

    TrainingReport trainNeat(
        InnovationTracker &tracker,
        int seed
    )
    {
        TrainingReport report;
        std::mt19937 rng(seed);

        tracker.reset(0);

        std::vector<Genome> population = initPop(tracker, rng);

        for (int gen = 0; gen < trainingConf::generations; gen++)
        {
            GenerationStats stats;
            Genome bestThisGen;

            std::vector<Genome> next = evolvePopulation(
                population,
                tracker,
                rng,
                &evaluateGenome,
                &stats,
                &bestThisGen);
            
            if (stats.bestFitness > report.bestFitness)
            {
                report.bestFitness = stats.bestFitness;
                report.bestGenome = bestThisGen;
            }

            report.generationsRun = gen + 1;

            std::cout
                << "[gen " << gen << "] "
                << "best=" << stats.bestFitness
                << " mean=" << stats.meanFitness
                << " species=" << stats.speciesCount
                << " overall=" << report.bestFitness
                << "\n";

            population = std::move(next);
        }

        return report;
    }
}