#include "utils.hpp"

sf::RectangleShape utils::createLine(sf::Vector2f start, sf::Vector2f end, float thickness)
{
    sf::Vector2f diff = end - start;
    float len = std::hypot(diff.x, diff.y);
    sf::RectangleShape line({thickness, len});
    line.setOrigin({thickness / 2.f, 0.f}); // Top-center origin
    line.setPosition(start);
    // Calculate angle: atan2(dy, dx) * 180 / PI - 90 (since rect points down)
    float angle = std::atan2(diff.y, diff.x) * 180.f / conf::PI - 90.f;
    line.setRotation(sf::degrees(angle));
    return line;
}

void utils::drawRail(sf::RenderTarget &target)
{
    // --- Rail from min to max pos ---
    sf::RectangleShape rail = utils::createLine(
        {conf::CART_PADDING, conf::WINDOW_SIZE.y / 2.f},
        {conf::WINDOW_SIZE.x - conf::CART_PADDING, conf::WINDOW_SIZE.y / 2.f},
        conf::RAIL_THICKNESS);
    rail.setFillColor(sf::Color::Transparent);
    rail.setOutlineColor(sf::Color::White);
    rail.setOutlineThickness(conf::RAIL_OUTLINE_THICKNESS);
    target.draw(rail);
}