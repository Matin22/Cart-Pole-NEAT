#pragma once

#include "../config.hpp"

#include <SFML/Graphics.hpp>
#include <array>

class Cart{
public:
    Cart();
    void draw(sf::RenderTarget& target);
    void update(float dt);

    void setAccelCommand(float a) { accelCommand = a; }
    float getAccelCommand() { return accelCommand; }

    sf::Vector2f getPos() {return cartCenter;}
    float getAcceleration() {return acceleration;}

    float getVelocity() { return velocity; }

    void reset(float x = conf::WINDOW_SIZE.x / 2.f, float v = 0.f);

private:
    sf::Texture texture = sf::Texture("../res/wheel.png");;
    std::array<sf::Sprite, 4> sprites =
    {
        sf::Sprite(texture),
        sf::Sprite(texture),
        sf::Sprite(texture),
        sf::Sprite(texture)
    };

    std::array<sf::Vector2f, 4> offsets =
    {
        sf::Vector2f{-conf::CART_OFFSET.x, conf::CART_OFFSET.y},            // bottom left
        sf::Vector2f{conf::CART_OFFSET.x, conf::CART_OFFSET.y},             // bottom right
        sf::Vector2f{conf::CART_OFFSET.x * 1.5f, -conf::CART_OFFSET.y},     // top right
        sf::Vector2f{-conf::CART_OFFSET.x * 1.5f, -conf::CART_OFFSET.y}     // top left
    };

    sf::Vector2f cartCenter;

    float leftEdge = 0.f;
    float rightEdge = 0.f;

    float velocity = 0.f;
    float velMax = conf::CART_VELOCITY_MAX;
    
    float acceleration = 0.f;
    float accelCommand = 0.f;
    float accelRate = conf::CART_ACCELERATION_RATE;
    
    float wheelAngle = 0.f;

    float min = conf::CART_PADDING;
    float max = conf::WINDOW_SIZE.x - conf::CART_PADDING;
};