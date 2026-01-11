#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>

#include "../config.hpp"

namespace utils
{
    sf::RectangleShape createLine(sf::Vector2f start, sf::Vector2f end, float thickness);
    void drawRail(sf::RenderTarget &target);
}