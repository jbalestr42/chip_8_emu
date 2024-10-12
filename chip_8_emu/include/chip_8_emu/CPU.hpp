#pragma once

#include <functional>
#include <stack>

class Chip8;
class Display;
class Input;
class Memory;

class CPU
{
public:
	CPU(Chip8& emulator);

	void initialize();
	bool tick();
	void updateTimers();

	bool drawThisFrame() { return _drawThisFrame; }
	void setDrawThisFrame(bool drawThisFrame) { _drawThisFrame = drawThisFrame; }
	bool isSoundTimerActive() { return _soundTimer > 0; }

	static const size_t MAX_REGISTER = 16;

private:
	class Instruction
	{
	public:
		Instruction(uint16_t mask, uint16_t code, std::function<void(uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1)> action) :
			mask(mask),
			code(code),
			action(action)
		{}

		// Mask and code are used to filter the opCode and determines the action to execute
		// Instruction is selected if (mask & opCode) == code
		uint16_t mask; 
		uint16_t code;

		// opCode: Current code being processed
		// b3: Byte at position 3 in the opCode
		// b2: Byte at position 2 in the opCode
		// b1: Byte at position 1 in the opCode
		std::function<void(uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1)> action;
	};

	void addInstruction(uint16_t mask, uint16_t code, std::function<void(uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1)> action);
	CPU::Instruction* getInstruction(uint16_t opCode);

	Chip8& _emulator;
	Memory& _memory;
	Display& _display;
	Input& _input;
	std::vector<CPU::Instruction> _instructions; // 35 opcodes which are all two bytes long and stored big-endian
	uint16_t _pc; // Program counter
	uint8_t _registers[CPU::MAX_REGISTER]; // Registers V0 - VF
	uint16_t _I; // Address register: 16 bit to store address
	std::stack<uint16_t> _stack;

	uint8_t _delayTimer;
	uint8_t _soundTimer;

	bool _drawThisFrame;
};
