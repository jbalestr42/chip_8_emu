#include "CPU.hpp"
#include "Chip8.hpp"
#include "Display.hpp"
#include "Memory.hpp"
#include <iostream>

CPU::CPU(Chip8& emulator) :
	_emulator(emulator),
	_display(emulator.display()),
	_memory(emulator.memory()),
	_pc(Chip8::ROM_START_ADDR),
	_I(0)
{
	memset(_registers, 0, CPU::MAX_REGISTER);
	_memory.clear();
}

void CPU::initialize()
{
	addInstruction(0x0000, 0x0FFF, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 0NNN: Unused
	});
	addInstruction(0xFFFF, 0x00E0, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 00E0: Clears the screen
		_display.clear();
	});
	addInstruction(0xF000, 0x1000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 1NNN: Jumps to address NNN
		_pc = NNN;
	});
	addInstruction(0xF000, 0x6000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 6XNN: Sets VX to NN
		_registers[X] = NN;
	});
	addInstruction(0xF000, 0x7000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 7XNN: Adds NN to VX
		_registers[X] += NN;
	});
	addInstruction(0xF000, 0xA000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// ANNN: Sets I to the address NNN
		_I = NNN;
	});
	addInstruction(0xF000, 0xD000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
		// Each row of 8 pixels is read as bit-coded starting from memory location I
		// I value does not change after the execution of this instruction.
		// As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen.

		uint8_t startX = _registers[X];
		uint8_t startY = _registers[Y];
		uint8_t height = N;

		_registers[0xF] = 0;

		for (uint8_t y = 0; y < height; y++)
		{
			uint8_t spriteY = _memory.read8(_I + y);

			// Sprite are always 8 pixels wide
			for (uint8_t x = 0; x < 8; x++)
			{
				uint8_t spritePixel = spriteY & (0x80 >> x);
				if (spritePixel)
				{
					// Wrap if going beyond screen boundaries
					uint8_t posX = (startX + x) % _display.width();
					uint8_t posY = (startY + y) % _display.height();
					bool isPixelOn = _display.isPixelOn(posX, posY);

					// Pixel is colliding so we set the flag
					if (isPixelOn)
					{
						_registers[0xF] = 1;
					}

					// Flip the pixel color
					_display.putPixel(posX, posY, !isPixelOn);
				}
			}
		}
	});
}

void CPU::addInstruction(uint16_t mask, uint16_t code, std::function<void(uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y)> execute)
{
	_instructions.emplace_back(CPU::Instruction(mask, code, execute));
}

const CPU::Instruction* CPU::getInstruction(uint16_t opCode) const
{
	for (size_t i = 0; i < _instructions.size(); i++)
	{
		const CPU::Instruction* instruction = &_instructions[i];
		if ((instruction->mask & opCode) == instruction->code)
		{
			return instruction;
		}
	}
	return nullptr;
}

bool CPU::tick()
{
	// Get the current opCode
	uint16_t opCode = (_memory.read8(_pc) << 8) | _memory.read8(_pc + 1);
	
	// Increment program counter
	_pc += 2;

	// Fetch the instruction
	const CPU::Instruction* instruction = getInstruction(opCode);

	// Execute the instruction
	if (instruction != nullptr)
	{
		// We preprocess opCode for convenience, they are not always used
		 // NNN: address
		uint16_t NNN = opCode & 0x0FFF;
		// NN: 8 bit constant
		uint8_t NN = opCode & 0x00FF;
		// N: 4 bit constant
		uint8_t N = opCode & 0x000F;
		// X and Y: 4 bit register identifier
		uint8_t X = (opCode & 0x0F00) >> 8;
		uint8_t Y = (opCode & 0x00F0) >> 4;
		instruction->execute(NNN, NN, N, X, Y);
	}
	else
	{
		std::cout << "[ERROR] Unknown opCode [" << std::hex << opCode << "]" << std::endl;
		return false;
	}

	return true;
}
