#pragma once

#include <SFML/Graphics.hpp>

class Display
{
public:
	Display(uint8_t width, uint8_t height, uint8_t pixelSize, const std::string& title);

	void display();
	void clear();
	void close();
	bool isOpen();
	void pollEvent();
	sf::Color getPixel(uint8_t x, uint8_t y);
	void putPixel(uint8_t x, uint8_t y, sf::Color color);
	uint8_t width() const { return _width; }
	uint8_t height() const { return _height; }

private:
	sf::RenderWindow _window;
	sf::VertexArray _vertices;

	uint8_t _width;
	uint8_t _height;
	uint8_t _pixelSize;
};
