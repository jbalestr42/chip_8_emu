#include "Display.hpp"

Display::Display(uint8_t width, uint8_t height, uint8_t pixelSize, const std::string& title) :
	_window(sf::VideoMode(width* pixelSize, height* pixelSize), title),
	_width(width),
	_height(height),
	_pixelSize(pixelSize),
	_pixelColorOff(sf::Color::Black),
	_pixelColorOn(sf::Color::White)
{
	_vertices.setPrimitiveType(sf::PrimitiveType::Quads);
	_vertices.resize(_width * _height * 4);

	for (uint8_t x = 0; x < _width; x++)
	{
		for (uint8_t y = 0; y < _height; y++)
		{
			sf::Vertex* quad = &_vertices[(x + y * _width) * 4];
			quad[0].position = sf::Vector2f(static_cast<float>(x * _pixelSize), static_cast<float>(y * _pixelSize));
			quad[1].position = sf::Vector2f(static_cast<float>((x + 1) * _pixelSize), static_cast<float>(y * _pixelSize));
			quad[2].position = sf::Vector2f(static_cast<float>((x + 1) * _pixelSize), static_cast<float>((y + 1) * _pixelSize));
			quad[3].position = sf::Vector2f(static_cast<float>(x * _pixelSize), static_cast<float>((y + 1) * _pixelSize));

			quad[0].color = _pixelColorOff;
			quad[1].color = _pixelColorOff;
			quad[2].color = _pixelColorOff;
			quad[3].color = _pixelColorOff;
		}
	}
}

void Display::display()
{
	_window.draw(_vertices);
	_window.display();
}

void Display::clear()
{
	for (uint16_t i = 0; i < _width * _height * 4; i++)
	{
		_vertices[i].color = _pixelColorOff;
	}
	_window.clear();
}

void Display::close()
{
	_window.close();
}

bool Display::isOpen() const
{
	return _window.isOpen();
}

void Display::pollEvent()
{
	sf::Event event;
	while (_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
		{
			close();
		}
	}
}

bool Display::isPixelOn(uint8_t x, uint8_t y) const
{
	return getPixel(x, y) == _pixelColorOn;
}

void Display::putPixel(uint8_t x, uint8_t y, bool isOn)
{
	putPixel(x, y, isOn ? _pixelColorOn : _pixelColorOff);
}

sf::Color Display::getPixel(uint8_t x, uint8_t y) const
{
	return _vertices[(x + y * _width) * 4].color;
}

void Display::putPixel(uint8_t x, uint8_t y, sf::Color color)
{
	sf::Vertex* quad = &_vertices[(x + y * _width) * 4];

	quad[0].color = color;
	quad[1].color = color;
	quad[2].color = color;
	quad[3].color = color;
}
