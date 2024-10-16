#pragma once

#include "Audio.hpp"
#include "CPU.hpp"
#include "Input.hpp"
#include "Memory.hpp"
#include "Display.hpp"
#include <string>

class Chip8
{
public:
	Chip8(size_t cyclesPerFrame);

	void initialize();
	void update();
	bool loadRom(const std::string& path);

	Display& display() { return _display; }
	Input& input() { return _input; }
	Memory& memory() { return _memory; }

	void setAudioEnabled(bool audioEnabled) { _audioEnabled = audioEnabled; }

	static const uint16_t FONT_START_ADDRESS = 0x050;
	static const uint16_t ROM_START_ADDR = 0x200;
	static const uint8_t SPRITE_WIDTH = 8;

private:
	void loadFont();

	Display _display;
	Memory _memory;
	Input _input;
	Audio _audio;
	CPU _cpu;

	// Configurable because some games may depends on it to run properly
	size_t _cyclesPerFrame;

	bool _audioEnabled;
};
