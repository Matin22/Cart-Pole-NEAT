#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

namespace conf
{
    // Window config
    inline constexpr sf::Vector2f const WINDOW_SIZE = {960.f, 720.f};
    inline constexpr uint32_t MAX_FRAMERATE = 240;
    inline constexpr float const dt = 1.0f / static_cast<float>(MAX_FRAMERATE);
    inline constexpr sf::State const window_state = sf::State::Windowed;

    // World Constants
    inline constexpr float GRAVITY = -981.0f;
    inline constexpr float PI = 3.14159265f;

    // Pendulum Config
    inline constexpr float PENDULUM_LENGTH = 200.f;
    inline constexpr float PENDULUM_FRICTION = 0.999;  // Very low friction - natural swing
    inline constexpr float PENDULUM_RADIUS = 15.f;
    inline constexpr float PENDULUM_THICKNESS = 5.f;
    inline constexpr sf::Color PENDULUM_COLOR = sf::Color(220, 100, 80);

    // Cart config
    inline constexpr float CART_WHEEL_RADIUS = 20.f;
    inline constexpr sf::Vector2f CART_OFFSET = {50.f, 30.f};
    inline constexpr float CART_PADDING = 75.f;
    inline constexpr float CART_FRICTION = 0.997f;       // PBD-style friction coefficient
    inline constexpr float CART_VELOCITY_MAX = 1000.f;    // Max cart velocity (units/sec)
    inline constexpr float CART_ACCELERATION_RATE = 3000.f;  // Max cart acceleration
    inline constexpr float CART_THICKNESS = 15.f;

    // Rail config
    inline constexpr float RAIL_THICKNESS = 15.f;
    inline constexpr float RAIL_OUTLINE_THICKNESS = 2.f;
}