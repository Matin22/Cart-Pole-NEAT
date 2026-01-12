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
    // Wrap angle to [-PI, PI]
    inline float wrapAngle(float theta)
    {
        while (theta > conf::PI) theta -= 2.f * conf::PI;
        while (theta < -conf::PI) theta += 2.f * conf::PI;
        return theta;
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
            float x0 = xCenter + smallDist(rng) * (0.2f * xHalfRange);
            float v0 = smallDist(rng) * 30.f;
            cart.reset(x0, v0);

            // Start pendulum at bottom (hanging down) with small perturbation
            float theta0 = 0.f + smallDist(rng) * 0.2f;
            float angularVel0 = smallDist(rng) * 0.5f;
            pendulum.reset(theta0, angularVel0);

            float fitness = 0.f;
            float timeBalanced = 0.f;
            
            std::vector<float> inputs(5, 0.f);
            bool earlyTerminate = false;

            for (int t = 0; t < trainingConf::maxStepsPerEpisode && !earlyTerminate; t++)
            {
                float x = cart.getPos().x;
                float vel = cart.getVelocity();
                float theta = pendulum.getAngle();
                float angularVel = pendulum.getAngularVel();

                float thetaWrapped = wrapAngle(theta);

                // Normalized inputs
                float xNorm = (x - xCenter) / xHalfRange;
                float velNorm = vel / conf::CART_VELOCITY_MAX;
                float angularVelNorm = angularVel / 10.f;

                // Boundary check
                if (x <= xMin || x >= xMax)
                {
                    fitness -= trainingConf::boundHitPenalty;
                    earlyTerminate = true;
                    break;
                }

                // Neural network inputs
                inputs[0] = xNorm;
                inputs[1] = velNorm;
                inputs[2] = std::sin(thetaWrapped);
                inputs[3] = std::cos(thetaWrapped);
                inputs[4] = angularVelNorm;

                std::vector<float> outputs = forwardPass(genome, inputs);
                float u = outputs.empty() ? 0.f : outputs[0];
                if (!std::isfinite(u)) u = 0.f;
                u = std::clamp(u, -1.f, 1.f);
                
                float accel = u * aMax;

                cart.setAccelCommand(accel);
                cart.update(conf::dt);
                pendulum.update(conf::dt, cart.getAcceleration());

                // Safety clamp on angular velocity
                if (!std::isfinite(pendulum.angularVel))
                    pendulum.angularVel = 0.f;
                else
                    pendulum.angularVel = std::clamp(pendulum.angularVel, -30.f, 30.f);

                // Reward calculation
                float cosTheta = std::cos(thetaWrapped);
                float heightReward = -cosTheta;  // +1 at top, -1 at bottom
                
                float posPenalty = xNorm * xNorm;
                float angVelPenalty = angularVelNorm * angularVelNorm;
                
                bool isUpright = (heightReward > 0.95f);
                bool isBalanced = isUpright && (std::abs(angularVelNorm) < 0.15f);
                
                if (isBalanced)
                    timeBalanced += conf::dt;

                float r = heightReward * 2.0f
                        - 0.3f * posPenalty
                        - 0.3f * angVelPenalty
                        + (isBalanced ? 3.0f : 0.f);
                
                r += 0.1f;
                r = std::clamp(r, -5.0f, 10.0f);

                if (!std::isfinite(r))
                    r = -10.f;

                fitness += r;
            }

            fitness += timeBalanced * 5.f;
            totalfitness += fitness;
        }

        return totalfitness / static_cast<float>(trainingConf::episodes);
    }
}