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
	Chip8(size_t cyclesPerFrame, bool saveLoadIncrement, bool vfReset, bool clipping, bool shifting, bool displayWait);

	void initialize();
	void update();
	bool loadRom(const std::string& path);

	Display& display() { return _display; }
	Input& input() { return _input; }
	Memory& memory() { return _memory; }

	bool isSaveLoadIncrementEnabled() { return _saveLoadIncrement; }
	bool isVfResetEnabled() { return _vfReset; }
	bool isClippingEnabled() { return _clipping; }
	bool isShiftingEnabled() { return _shifting; }
	bool isDisplayWaitEnabled() { return _displayWait; }

	static const uint16_t FONT_START_ADDRESS = 0x050;
	static const uint16_t ROM_START_ADDR = 0x200;

private:
	void loadFont();
	
	Display _display;
	Memory _memory;
	Input _input;
	Audio _audio;
	CPU _cpu;

	// These value are configurable because some games may depends on it to run properly
	size_t _cyclesPerFrame;
	bool _saveLoadIncrement;
	bool _vfReset;
	bool _clipping;
	bool _shifting;
	bool _displayWait;
};
