#pragma once

#include "forwardpass.hpp"
#include "../environment/cart.hpp"
#include "../environment/pendulum.hpp"

#include "../neatConfig.hpp"
#include "../config.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

namespace neat
{
    // Global curriculum state - tracks current difficulty
    inline int g_currentGeneration = 0;
    
    // Wrap angle to [-PI, PI]
    inline float wrapAngle(float theta)
    {
        while (theta > conf::PI) theta -= 2.f * conf::PI;
        while (theta < -conf::PI) theta += 2.f * conf::PI;
        return theta;
    }

    // Get curriculum phase index (0 to curriculumPhases-1)
    inline int getCurriculumPhase(int generation)
    {
        int totalGens = trainingConf::generations;
        int numPhases = trainingConf::curriculumPhases;
        float fraction = trainingConf::phaseFraction;
        
        int gensPerPhase = static_cast<int>(totalGens * fraction);
        if (gensPerPhase < 1) gensPerPhase = 1;
        
        int phase = generation / gensPerPhase;
        return std::min(phase, numPhases - 1);
    }

    // Get curriculum difficulty (0 = easiest/top, 1 = hardest/bottom)
    inline float getCurriculumDifficulty(int generation)
    {
        int phase = getCurriculumPhase(generation);
        int numPhases = trainingConf::curriculumPhases;
        
        if (numPhases <= 1) return 0.0f;  // Single phase = top (easiest)
        return static_cast<float>(phase) / static_cast<float>(numPhases - 1);
    }

    // Get starting angle based on curriculum progress
    inline float getCurriculumStartAngle(int generation, std::mt19937 &rng)
    {
        std::uniform_real_distribution<float> noise(-0.15f, 0.15f);  // Small noise
        
        float difficulty = getCurriculumDifficulty(generation);
        float baseAngle = conf::PI * (1.0f - difficulty);
        
        return baseAngle + noise(rng);
    }

    float evaluateGenome(
        Genome &genome,
        std::mt19937 &rng
    )
    {
        std::uniform_real_distribution<float> smallDist(-0.5f, 0.5f);

        float xMin = conf::CART_PADDING;
        float xMax = conf::WINDOW_SIZE.x - conf::CART_PADDING;
        float xCenter = conf::WINDOW_SIZE.x / 2.f;
        float xHalfRange = (xMax - xMin) / 2.f;

        float aMax = conf::CART_ACCELERATION_RATE;

        float totalfitness = 0.f;

        for (int ep = 0; ep < trainingConf::episodes; ep++)
        {
            Cart cart;
            Pendulum pendulum;

            // Start near center with small random offset
            float x0 = xCenter + smallDist(rng) * (0.15f * xHalfRange);
            float v0 = smallDist(rng) * 20.f;  // Small initial velocity
            cart.reset(x0, v0);

            // Start angle from curriculum + small perturbation
            float theta0 = getCurriculumStartAngle(g_currentGeneration, rng);
            float angularVel0 = smallDist(rng) * 0.3f;  // Small angular velocity
            pendulum.reset(theta0, angularVel0);

            float fitness = 0.f;
            float timeBalanced = 0.f;
            int stepsBalanced = 0;
            std::vector<float> inputs(5, 0.f);

            for (int t = 0; t < trainingConf::maxStepsPerEpisode; t++)
            {
                float x = cart.getPos().x;
                float vel = cart.getVelocity();
                float theta = pendulum.getAngle();
                float angularVel = pendulum.getAngularVel();

                float thetaWrapped = wrapAngle(theta);

                // Normalized inputs for neural network
                float xNorm = (x - xCenter) / xHalfRange;        // [-1, 1]
                float velNorm = vel / conf::CART_VELOCITY_MAX;   // ~[-1, 1]
                float angularVelNorm = angularVel / 8.f;         // Normalized angular vel

                // Boundary check
                if (x <= xMin || x >= xMax)
                {
                    fitness -= trainingConf::boundHitPenalty;
                    break;
                }

                // Neural network inputs
                inputs[0] = xNorm;
                inputs[1] = velNorm;
                inputs[2] = std::sin(thetaWrapped);  // Continuous angle representation
                inputs[3] = std::cos(thetaWrapped);
                inputs[4] = angularVelNorm;

                std::vector<float> outputs = forwardPass(genome, inputs);
                float u = outputs.empty() ? 0.f : outputs[0];
                u = std::clamp(u, -1.f, 1.f);
                float accel = u * aMax;

                cart.setAccelCommand(accel);
                cart.update(conf::dt);
                pendulum.update(conf::dt, cart.getAcceleration());

                // === REWARD CALCULATION ===
                float cosTheta = std::cos(thetaWrapped);
                float sinTheta = std::sin(thetaWrapped);
                
                // Upright reward: +1 when perfectly up, -1 when down
                float upright = -cosTheta;
                
                // Squared penalties (smooth gradients)
                float anglePenalty = sinTheta * sinTheta;           // 0 at top/bottom, 1 at horizontal
                float angVelPenalty = angularVelNorm * angularVelNorm;
                float posPenalty = xNorm * xNorm;
                float velPenalty = velNorm * velNorm;
                
                // Check if balanced (within ~15 degrees of top and nearly still)
                bool isBalanced = (upright > 0.96f) && (std::abs(angularVelNorm) < 0.1f);
                
                if (isBalanced)
                {
                    stepsBalanced++;
                    timeBalanced += conf::dt;
                }
                
                // Reward: heavily reward being upright and still
                float r = 1.0f                          // Base survival reward
                        + 2.0f * upright                // -2 to +2 based on angle
                        + (isBalanced ? 3.0f : 0.f)     // Big bonus for being balanced
                        - 0.5f * angVelPenalty          // Penalize spinning
                        - 0.2f * posPenalty             // Stay centered
                        - 0.1f * velPenalty;            // Don't move too fast

                fitness += r;
            }

            // Bonus for sustained balance
            fitness += stepsBalanced * 0.5f;  // Extra reward per balanced step
            fitness += timeBalanced * 10.f;   // Bonus for total balanced time

            totalfitness += fitness;
        }

        return totalfitness / static_cast<float>(trainingConf::episodes);
    }
}