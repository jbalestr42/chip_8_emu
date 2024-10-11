#include "Chip8.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Provide the rom as first argument." << std::endl;
		return 0;
	}

	Chip8 emulator(60, true, true, true, true, true);
	// TODO: crt effect for rendering

	//if (emulator.loadRom("C:\\Users\\julie\\Documents\\GitHub\\chip_8_emu\\roms\\tests\\1-chip8-logo.ch8"))
	//if (emulator.loadRom("C:\\Users\\julie\\Documents\\GitHub\\chip_8_emu\\roms\\tests\\2-ibm-logo.ch8"))
	//if (emulator.loadRom("C:\\Users\\julie\\Documents\\GitHub\\chip_8_emu\\roms\\tests\\3-corax+.ch8"))
	//if (emulator.loadRom("C:\\Users\\julie\\Documents\\GitHub\\chip_8_emu\\roms\\tests\\4-flags.ch8"))
	//if (emulator.loadRom("C:\\Users\\julie\\Documents\\GitHub\\chip_8_emu\\roms\\tests\\5-quirks.ch8"))
	//if (emulator.loadRom("C:\\Users\\julie\\Documents\\GitHub\\chip_8_emu\\roms\\tests\\6-keypad.ch8"))
	//if (emulator.loadRom("C:\\Users\\julie\\Documents\\GitHub\\chip_8_emu\\roms\\tests\\7-beep.ch8"))
	//if (emulator.loadRom("C:\\Users\\julie\\Documents\\GitHub\\chip_8_emu\\roms\\games\\BREAKOUT.ch8"))
	//if (emulator.loadRom("C:\\Users\\julie\\Documents\\GitHub\\chip_8_emu\\roms\\games\\PONG.ch8"))
	//if (emulator.loadRom("C:\\Users\\julie\\Documents\\GitHub\\chip_8_emu\\roms\\games\\TETRIS.ch8"))
	if (emulator.loadRom(argv[1]))
	{
		emulator.initialize();
		emulator.update();
	}
	else
	{
		std::cout << "[ERROR] An error occured while loading the rom '" << argv[1] << "'" << std::endl;
	}

	return 0;
}
