#pragma once

#include "config.hpp"

namespace neat::mutConf
{
    // WEIGHT MUTATION
    
    // Per-connection probabilities
    inline constexpr float pPerturb = 0.90f;  // Higher - fine-tune weights more often
    inline constexpr float pReset   = 0.05f;  // Lower - preserve good weights

    // Distributions
    inline constexpr float perturbStd = 0.15f;  // Smaller perturbations for fine control
    inline constexpr float weightMin  = -2.0f;  // Wider range for more expressive networks
    inline constexpr float weightMax  =  2.0f;

    // Mutate disabled connections too?
    inline constexpr bool mutateDisabled = false;

    // CONNECTION ADD MUTATION
    
    // Add-connection mutation
    inline constexpr int addConnMaxTries = 64;
    inline constexpr bool cycleCheckEnabledOnly = true;

    // CROSSOVER
    inline constexpr float kDisableInheritProb = 0.75f;
};

namespace neat::specConf
{
    // SPECIATION - tuned for small networks
    
    // compatibility distance:
    // delta = (c1*E)/N + (c2*D)/N + c3*W
    inline constexpr float c1 = 1.0f;   // excess coefficient
    inline constexpr float c2 = 1.0f;   // disjoint coefficient
    inline constexpr float c3 = 0.3f;   // weight diff - lower to allow more diversity

    inline constexpr int smallGenomeCutoff = 20;
    inline constexpr float threshold = 3.0f;  // Species threshold
};

namespace neat::evoConf
{
    // POPULATION - larger for better exploration
    inline constexpr int populationSize = 200;

    // SELECTION
    inline constexpr int tournamentK = 3;
    inline constexpr int elitesPerSpecies = 2;  // Preserve best solutions

    // OPERATOR RATES - optimized for balancing
    inline constexpr float pCrossover = 0.75f;
    inline constexpr float pMutateWeights = 0.80f;  // High - balancing needs weight tuning
    inline constexpr float pAddConnection = 0.08f;  // Lower - minimal network is often best
    inline constexpr float pAddNode = 0.03f;        // Very low - simple networks work well

    inline constexpr bool useFitnessSharing = true;
};

namespace neat::trainingConf
{
    // TRAINING - optimized for balancing from top
    inline constexpr int generations = 1;  // Enough to converge
    inline constexpr int maxStepsPerEpisode = conf::MAX_FRAMERATE * 10;  // 8 seconds per episode
    inline constexpr int episodes = 3;  // Multiple episodes for robust evaluation
    inline constexpr float boundHitPenalty = 500.f;  // Strong penalty for hitting walls
    
    // CURRICULUM - start from top only for balancing
    inline constexpr int curriculumPhases = 1;  // Single phase = top only
    inline constexpr float phaseFraction = 1.0f;
};