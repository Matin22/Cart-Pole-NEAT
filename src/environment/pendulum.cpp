#include "pendulum.hpp"

#include <cmath>
#include <array>

#include "../config.hpp"
#include "utils.hpp"

Pendulum::Pendulum(){
    pendPoint.setOrigin({radius, radius});
    pendPoint.setFillColor(conf::PENDULUM_COLOR);
    pendPoint.setOutlineColor(sf::Color::White);
    pendPoint.setOutlineThickness(5.f);

    cartPoint.setOrigin({radius, radius});
    cartPoint.setFillColor(conf::PENDULUM_COLOR);
    cartPoint.setOutlineColor(sf::Color::White);
    cartPoint.setOutlineThickness(5.f);
}

void Pendulum::update(float dt, float cartAccel)
{
    float angularAccel = (conf::GRAVITY * std::sin(angle) - cartAccel * std::cos(angle)) / length;

    angularVel += angularAccel * dt;
    angle += angularVel * dt;

    angularVel *= conf::PENDULUM_FRICTION;
}

void Pendulum::draw(sf::RenderTarget &target, sf::Vector2f cartPos)
{
    sf::Vector2f endPoint{
        cartPos.x + length * std::sin(angle),
        cartPos.y + length * std::cos(angle)};

    sf::RectangleShape handle = utils::createLine(cartPos, endPoint, conf::PENDULUM_THICKNESS);

    target.draw(handle);

    pendPoint.setPosition(endPoint);
    cartPoint.setPosition(cartPos);
    target.draw(pendPoint);
    target.draw(cartPoint);
}