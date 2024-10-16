#pragma once

class Chip8;
class Display;
class Memory;

class CPU
{
public:
	CPU(Chip8& emulator);

	void tick();

private:
	Chip8& _emulator;
	Memory& _memory;
	Display& _display;
};
