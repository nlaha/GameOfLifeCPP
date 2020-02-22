#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>

sf::Color HsvToRgb(sf::Color hsv);

sf::Color RgbToHsv(sf::Color rgb);