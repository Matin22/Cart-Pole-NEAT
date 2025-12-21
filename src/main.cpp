#include <SFML/Graphics.hpp>

#include "events.hpp"
#include "config.hpp"

int main()
{
    auto window = sf::RenderWindow(sf::VideoMode(conf::WINDOW_SIZE), "CMake SFML Project");
    window.setFramerateLimit(144);

    while (window.isOpen())
    {
        processEvents(window);

        window.clear();
        window.display();
    }
}
