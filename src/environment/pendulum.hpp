#pragma once

#include <SFML/Graphics.hpp>
#include "../config.hpp"

class Pendulum
{
private:

public:
    Pendulum();
    void update(float dt, float cartAccel);
    void draw(sf::RenderTarget &target, sf::Vector2f cartPos);
    void reset(float a = 0.f, float av = 0.f) { angle = a; angularVel = av; }

    float getAngle() { return angle; }
    float getAngularVel() { return angularVel; }

    float length = conf::PENDULUM_LENGTH;
    float radius = conf::PENDULUM_RADIUS;
    float angle = 0.f;
    float angularVel = 0.f;

    sf::CircleShape pendPoint = sf::CircleShape(radius);
    sf::CircleShape cartPoint = sf::CircleShape(radius);
};