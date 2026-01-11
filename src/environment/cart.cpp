#include "cart.hpp"
#include "../config.hpp"
#include "utils.hpp"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window.hpp>
#include <algorithm>
#include <cmath>

Cart::Cart()
{
    
    sf::Vector2f texSize = {static_cast<sf::Vector2f>(texture.getSize())};
    sf::Vector2f scale = {conf::CART_WHEEL_RADIUS * 2.f / texSize.x, conf::CART_WHEEL_RADIUS * 2.f / texSize.y};
    cartCenter = conf::WINDOW_SIZE / 2.f;

    for (int i = 0; i < sprites.size(); i++)
    {
        sprites[i].setScale(scale);
        sprites[i].setOrigin(texSize / 2.f);
        sprites[i].setPosition(cartCenter + offsets[i]);
    }
        
}

void Cart::draw(sf::RenderTarget& target)
{
    sf::Vector2f posBL = sprites[0].getPosition();
    sf::Vector2f posBR = sprites[1].getPosition();
    sf::Vector2f posTR = sprites[2].getPosition();
    sf::Vector2f posTL = sprites[3].getPosition();

    // --- Left Rectangle ---
    sf::RectangleShape leftRect = utils::createLine(posTL, posBL, conf::CART_THICKNESS);
    
    // --- Right Rectangle ---
    sf::RectangleShape rightRect = utils::createLine(posTR, posBR, conf::CART_THICKNESS);

    // --- Middle Rectangle ---
    sf::Vector2f midLeft = (posTL + posBL) * 0.5f;
    sf::Vector2f midRight = (posTR + posBR) * 0.5f;
    sf::RectangleShape midRect = utils::createLine(midLeft, midRight, conf::CART_THICKNESS);

    target.draw(leftRect);
    target.draw(rightRect);
    target.draw(midRect);
    
    for (auto &sprite : sprites){
        target.draw(sprite);
    }
}

void Cart::update(float dt)
{
    acceleration = std::clamp(accelCommand, -accelRate, accelRate);

    velocity += acceleration * dt;
    if (acceleration == 0.f) velocity *= conf::CART_FRICTION;

    if (velocity > velMax){velocity = velMax;}
    if (velocity < -velMax){velocity = -velMax;}
    
    float dx = velocity * dt;
    cartCenter.x += dx;

    leftEdge = cartCenter.x + offsets[3].x;
    rightEdge = cartCenter.x + offsets[2].x;

    if (leftEdge < min) {
        cartCenter.x = min - offsets[3].x;
        velocity = 0.f;
        acceleration = 0.f;
    }
    if (rightEdge > max) {
        cartCenter.x = max - offsets[2].x;
        velocity = 0.f;
        acceleration = 0.f;
    }

    wheelAngle -= (dx * 180.f) / (conf::CART_WHEEL_RADIUS * conf::PI);

    for (int i = 0; i < sprites.size(); i++) {
        sprites[i].setPosition(cartCenter + offsets[i]);

        float rotation = (i < 2) ? wheelAngle : -wheelAngle;
        sprites[i].setRotation(sf::degrees(rotation));
    }
    
}

void Cart::reset(float x, float v)
{
    cartCenter = {x, conf::WINDOW_SIZE.y / 2.f};

    velocity = v;
    acceleration = 0.f;
    accelCommand = 0.f;
    wheelAngle = 0.f;

    leftEdge = cartCenter.x + offsets[3].x;
    rightEdge = cartCenter.x + offsets[2].x;

    for (int i = 0; i < sprites.size(); i++)
    {
        sprites[i].setPosition(cartCenter + offsets[i]);
        sprites[i].setRotation(sf::degrees(0.f));
    }
}
