#include "Chip8.hpp"
#include <SFML/System/Clock.hpp>
#include <fstream>

Chip8::Chip8(uint8_t pixelSize, size_t cyclesPerFrame) :
	_display(64, 32, pixelSize, "CHIP 8"),
	_memory(),
	_cpu(*this),
	_cyclesPerFrame(cyclesPerFrame)
{ }

void Chip8::update()
{
	sf::Clock frameTimer;
	const float frameDuration = 1.f / 60.f;

	while (_display.isOpen())
	{
		_display.pollEvent();

		frameTimer.restart();
		for (size_t i = 0; i < _cyclesPerFrame; i++)
		{
			_cpu.tick();
		}

		// Wait to reach 60fps
		sf::sleep(sf::seconds(std::max(frameDuration - frameTimer.getElapsedTime().asSeconds(), 0.f)));

		_display.display();
	}
}

bool Chip8::loadRom(const std::string& path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);

	if (file.is_open())
	{
		file.seekg(0, std::ios_base::end);
		auto length = file.tellg();
		file.seekg(0, std::ios_base::beg);

		if (length > 0)
		{
			std::vector<std::uint8_t> buffer(length);
			file.read(reinterpret_cast<char*>(buffer.data()), length);

			_memory.copyBuffer(Chip8::ROM_START_ADDR, &buffer[0], length);

			file.close();
	
			return true;
		}
	}

	return false;
}
