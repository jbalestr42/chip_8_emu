#pragma once

#include <functional>

class Chip8;
class Display;
class Memory;

class CPU
{
public:
	CPU(Chip8& emulator);

	void initialize();
	bool tick();

	static const size_t MAX_REGISTER = 16;

private:
	class Instruction
	{
	public:
		Instruction(uint16_t mask, uint16_t code, std::function<void(uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y)> execute) :
			mask(mask),
			code(code),
			execute(execute)
		{}

		// Mask and code are used to filter the opCode and determines the action to execute
		// Instruction is selected if (mask & opCode) == code
		uint16_t mask; 
		uint16_t code;

		std::function<void(uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y)> execute;
	};

	void addInstruction(uint16_t mask, uint16_t code, std::function<void(uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y)> execute);
	const CPU::Instruction* getInstruction(uint16_t opCode) const;

	Chip8& _emulator;
	Memory& _memory;
	Display& _display;
	std::vector<CPU::Instruction> _instructions;
	uint16_t _pc;
	uint8_t _registers[CPU::MAX_REGISTER];
	uint16_t _I;
};
