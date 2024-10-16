#pragma once

#include <SFML/Graphics.hpp>

class Display
{
public:
	Display(uint8_t width, uint8_t height, uint8_t pixelSize, const std::string& title);

	void display();
	void clear();
	void close();
	bool isOpen() const;
	void pollEvent();

	uint8_t width() const { return _width; }
	uint8_t height() const { return _height; }

	bool isPixelOn(uint8_t x, uint8_t y) const;
	void putPixel(uint8_t x, uint8_t y, bool isOn);

	sf::Color getPixel(uint8_t x, uint8_t y) const;
	void putPixel(uint8_t x, uint8_t y, sf::Color color);

	void setPixelColorOff(sf::Color color) { _pixelColorOff = color; }
	void setPixelColorOn(sf::Color color) { _pixelColorOn = color; }

private:
	sf::RenderWindow _window;
	sf::VertexArray _vertices;

	uint8_t _width;
	uint8_t _height;
	uint8_t _pixelSize;
	sf::Color _pixelColorOff;
	sf::Color _pixelColorOn;
};
