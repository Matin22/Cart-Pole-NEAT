#pragma once

#include "config.hpp"

namespace neat::mutConf
{
    // WEIGHT MUTATION - Based on NEAT best practices
    
    // Per-connection probabilities (applied per-connection during mutation)
    inline constexpr float pPerturb = 0.80f;  // 80% chance of small perturbation
    inline constexpr float pReset   = 0.10f;  // 10% chance of complete reset
    // Remaining 10% - no change

    // Distributions - wider range for better exploration
    inline constexpr float perturbStd = 0.3f;   // Larger perturbations for faster learning
    inline constexpr float perturbSmall = 0.05f; // Small perturbations for fine-tuning
    inline constexpr float weightMin  = -3.0f;  // Wider range for expressive networks
    inline constexpr float weightMax  =  3.0f;

    // Mutate disabled connections too?
    inline constexpr bool mutateDisabled = false;

    // CONNECTION ADD MUTATION
    inline constexpr int addConnMaxTries = 100;  // More attempts for sparse networks
    inline constexpr bool cycleCheckEnabledOnly = true;

    // CROSSOVER
    inline constexpr float kDisableInheritProb = 0.75f;
    
    // Mutation count per genome (parametric mutations)
    inline constexpr int mutCount = 3;  // Multiple weight/bias mutations per generation
};

namespace neat::specConf
{
    // SPECIATION - tuned for NEAT best practices
    
    // compatibility distance:
    // delta = (c1*E)/N + (c2*D)/N + c3*W
    inline constexpr float c1 = 1.0f;   // excess coefficient
    inline constexpr float c2 = 1.0f;   // disjoint coefficient
    inline constexpr float c3 = 0.4f;   // weight diff - balance topology vs weights

    inline constexpr int smallGenomeCutoff = 20;
    inline constexpr float threshold = 2.5f;  // Lower threshold for more species diversity
    inline constexpr int minSpecies = 5;      // Maintain minimum diversity
    inline constexpr int maxSpecies = 25;     // Cap species for efficiency
    
    // Threshold adaptation rates
    inline constexpr float thresholdDelta = 0.3f;  // How much to adjust threshold
};

namespace neat::evoConf
{
    // POPULATION - larger for better exploration (guide recommends 1000)
    inline constexpr int populationSize = 1000;  // Balanced for performance vs exploration

    // SELECTION
    inline constexpr int tournamentK = 3;
    inline constexpr int elitesPerSpecies = 3;   // Preserve more top solutions
    inline constexpr float eliteRatio = 0.20f;   // Top 20% preserved without mutation

    // OPERATOR RATES - based on NEAT paper recommendations
    inline constexpr float pCrossover = 0.75f;
    inline constexpr float pMutateWeights = 0.80f;  // High - weight tuning is critical
    inline constexpr float pAddConnection = 0.30f;  // Higher - networks need connections
    inline constexpr float pAddNode = 0.05f;        // 5% - allows complexity growth

    inline constexpr bool useFitnessSharing = true;
    
    // Stagnation detection
    inline constexpr int stagnationGens = 15;   // Generations without improvement
    inline constexpr float stagnationPenalty = 0.5f;  // Reduce offspring allocation
};

namespace neat::trainingConf
{
    // TRAINING
    inline constexpr int generations = 15;
    inline constexpr int maxStepsPerEpisode = conf::MAX_FRAMERATE * 30;  // 30 second episodes
    inline constexpr int episodes = 5;
    inline constexpr float boundHitPenalty = 500.f;
};