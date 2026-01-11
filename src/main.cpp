
#include "environment/cart.hpp"
#include "environment/events.hpp"
#include "environment/pendulum.hpp"
#include "environment/utils.hpp"

#include "config.hpp"

#include "agent/forwardpass.hpp"
#include "agent/training.hpp"
#include "agent/innovationTracker.hpp"
#include "agent/serialization.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <cmath>
#include <algorithm>
#include <iostream>
#include <random>
#include <chrono>

const std::string GENOME_FILE = "../res/best_genome.bin";

int main()
{
    neat::Genome genome;
    
    // Random seed based on current time
    int seed = static_cast<int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::cout << "Using seed: " << seed << "\n";

    // Try to load existing genome, otherwise train a new one
    if (neat::genomeFileExists(GENOME_FILE))
    {
        std::cout << "Found saved genome. Loading...\n";
        if (!neat::loadGenome(genome, GENOME_FILE))
        {
            std::cerr << "Failed to load genome. Training new one...\n";
            neat::InnovationTracker tracker;
            neat::TrainingReport report = neat::trainNeat(tracker, seed);
            genome = report.bestGenome;
            neat::saveGenome(genome, GENOME_FILE);
        }
    }
    else
    {
        std::cout << "No saved genome found. Training...\n";
        neat::InnovationTracker tracker;
        neat::TrainingReport report = neat::trainNeat(tracker, seed);
        genome = report.bestGenome;
        std::cout << "Training finished. Best fitness = " << report.bestFitness
                  << " after " << report.generationsRun << " generations.\n";
        neat::saveGenome(genome, GENOME_FILE);
    }

    auto window = sf::RenderWindow(sf::VideoMode(static_cast<sf::Vector2u>(conf::WINDOW_SIZE)), "Pendulum Project", sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(144);

    float accelCmd = 0.f;
    
    sf::Clock clock;
    float accumulator = 0.f;
    
    Cart myCart = Cart();
    Pendulum myPendulum = Pendulum();
    // myPendulum.reset(0.f, 0.f);  // Start pendulum hanging down (theta = 0)
    myPendulum.reset(conf::PI, 0.f);
    
    bool useAI = false;
    bool prevT = false;

    float xMin = conf::CART_PADDING;
    float xMax = conf::WINDOW_SIZE.x - conf::CART_PADDING;
    float xCenter = conf::WINDOW_SIZE.x / 2.f;
    float xHalfRange = (xMax - xMin) / 2.f;

    while (window.isOpen())
    {
        processEvents(window);

        float frameDt = clock.restart().asSeconds();
        frameDt = std::min(frameDt, 0.25f);
        accumulator += frameDt;

        bool nowT = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::T);
        if (nowT && !prevT) useAI = !useAI;
        prevT = nowT;

        accelCmd = 0.f;
        if (!useAI)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
                accelCmd -= conf::CART_ACCELERATION_RATE;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
                accelCmd += conf::CART_ACCELERATION_RATE;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
            {
                myCart.reset();
                // myPendulum.reset(0.f, 0.f);  // Reset pendulum hanging down
                myPendulum.reset(conf::PI, 0.f);
            }
        }
        else
        {
            float x = myCart.getPos().x;
            float vel = myCart.getVelocity();
            float theta = myPendulum.getAngle();
            float angularVel = myPendulum.getAngularVel();

            // Wrap angle to [-PI, PI] for consistent input
            while (theta > conf::PI) theta -= 2.f * conf::PI;
            while (theta < -conf::PI) theta += 2.f * conf::PI;

            float xNorm = (x - xCenter) / xHalfRange;
            float velNorm = vel / conf::CART_VELOCITY_MAX;
            float angularVelNorm = angularVel / 10.f;

            std::vector<float> inputs{
                xNorm,
                velNorm,
                std::sin(theta),
                std::cos(theta),
                angularVelNorm
            };

            auto out = neat::forwardPass(genome, inputs);
            float u = out.empty() ? 0.f : out[0];
            u = std::clamp(u, -1.f, 1.f);

            accelCmd = u * conf::CART_ACCELERATION_RATE;
        }

        window.clear(sf::Color(50, 50, 50));

        while (accumulator >= conf::dt)
        {
            myCart.setAccelCommand(accelCmd);
            myCart.update(conf::dt);
            myPendulum.update(conf::dt, myCart.getAcceleration());

            accumulator -= conf::dt;
        }

        utils::drawRail(window);
        myCart.draw(window);
        myPendulum.draw(window, myCart.getPos());

        window.display();
    }
}

