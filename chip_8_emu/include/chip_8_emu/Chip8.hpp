#pragma once

#include "CPU.hpp"
#include "Memory.hpp"
#include "Display.hpp"
#include <string>

class Chip8
{
public:
	Chip8(uint8_t pixelSize, size_t cyclesPerFrame);

	void initialize();
	void update();
	bool loadRom(const std::string& path);

	Display& display() { return _display; }
	Memory& memory() { return _memory; }

	static const uint16_t ROM_START_ADDR = 0x200;

private:
	Display _display;
	Memory _memory;
	CPU _cpu;

	// Configurable because some games may depends on it to run properly
	size_t _cyclesPerFrame;
};
