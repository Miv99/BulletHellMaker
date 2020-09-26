#pragma once
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <TGUI/Transform.hpp>
#include <TGUI/RenderStates.hpp>

sf::RenderStates toSFMLRenderStates(tgui::RenderStates renderStates);

sf::Vector2f operator+(const sf::Vector2f v1, const tgui::Vector2f v2);
sf::Vector2f operator-(const sf::Vector2f v1, const tgui::Vector2f v2);