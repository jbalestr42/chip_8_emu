#include "CPU.hpp"
#include "Chip8.hpp"
#include "Display.hpp"
#include "Memory.hpp"

CPU::CPU(Chip8& emulator) :
	_emulator(emulator),
	_display(emulator.display()),
	_memory(emulator.memory())
{
	_memory.clear();
}

void CPU::tick()
{
}
