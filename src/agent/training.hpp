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
        
        int lastPhase = -1;
        float bestFitnessThisPhase = -1e30f;

        for (int gen = 0; gen < trainingConf::generations; gen++)
        {
            // Update curriculum - this controls starting angle difficulty
            g_currentGeneration = gen;
            int currentPhase = getCurriculumPhase(gen);
            
            // When entering a new curriculum phase, reset phase-best tracking
            // and re-evaluate the overall best genome on the new difficulty
            if (currentPhase != lastPhase)
            {
                bestFitnessThisPhase = -1e30f;
                
                // Re-evaluate overall best on new phase to give it a fair comparison
                if (report.bestFitness > -1e20f)
                {
                    float reEvalFitness = evaluateGenome(report.bestGenome, rng);
                    std::cout << "  [Phase transition: re-evaluated best genome: " 
                              << reEvalFitness << "]\n";
                    
                    // Inject the best genome back into population to compete
                    if (!population.empty())
                    {
                        population[0] = report.bestGenome;
                    }
                }
                lastPhase = currentPhase;
            }
            
            GenerationStats stats;
            Genome bestThisGen;

            std::vector<Genome> next = evolvePopulation(
                population,
                tracker,
                rng,
                &evaluateGenome,
                &stats,
                &bestThisGen);
            
            // Track best in this curriculum phase
            if (stats.bestFitness > bestFitnessThisPhase)
            {
                bestFitnessThisPhase = stats.bestFitness;
            }
            
            // Update overall best - but compare fairly within same evaluation context
            // The fitness is already scaled by curriculum difficulty in evaluateGenome
            if (stats.bestFitness > report.bestFitness)
            {
                report.bestFitness = stats.bestFitness;
                report.bestGenome = bestThisGen;
            }

            report.generationsRun = gen + 1;

            // Show curriculum phase using configurable system
            float difficulty = getCurriculumDifficulty(gen);
            float startAngleDeg = 180.f * (1.0f - difficulty);  // For display

            std::cout
                << "[gen " << gen << " | ~" << static_cast<int>(startAngleDeg) << "] "
                << "best=" << stats.bestFitness
                << " phaseBest=" << bestFitnessThisPhase
                << " mean=" << stats.meanFitness
                << " species=" << stats.speciesCount
                << " overallBest=" << report.bestFitness
                << "\n";

            population = std::move(next);
        }

        return report;
    }
}