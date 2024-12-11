#include "Chip8.hpp"
#include <SFML/System/Clock.hpp>
#include <iostream>
#include <fstream>

Chip8::Chip8(size_t cyclesPerFrame) :
	_display(64, 32, 16, "CHIP 8"),
	_memory(),
	_input(),
	_audio(),
	_cpu(*this),
	_cyclesPerFrame(cyclesPerFrame),
	_audioEnabled(true)
{ }

void Chip8::initialize()
{
	loadFont();
	_cpu.initialize();
}

void Chip8::update()
{
	sf::Clock frameTimer;
	bool isRunning = true;
	const float frameDuration = 1.f / 60.f;

	while (_display.isOpen() && isRunning)
	{
		_display.pollEvent();

		_input.tick();

		frameTimer.restart();
		for (size_t i = 0; i < _cyclesPerFrame; i++)
		{
			if (!_cpu.tick())
			{
				// An error occured, stop execution
				isRunning = false;
				break;
			}
		}

		// Wait to reach 60fps
		sf::sleep(sf::seconds(std::max(frameDuration - frameTimer.getElapsedTime().asSeconds(), 0.f)));

		_display.display();

		if (_audioEnabled)
		{
			// Play audio before we update the timer
			if (_cpu.isSoundTimerActive())
			{
				_audio.playSound();
			}
			else
			{
				_audio.stopSound();
			}
		}

		// Update timer once per frame
		_cpu.updateTimers();
	}
}

void Chip8::loadFont()
{
	std::vector<uint8_t> fontData =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	_memory.copyBuffer(Chip8::FONT_START_ADDRESS, &fontData[0], fontData.size());
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
