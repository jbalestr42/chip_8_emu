#include "CPU.hpp"
#include "Chip8.hpp"
#include "Display.hpp"
#include "Input.hpp"
#include "Memory.hpp"
#include <iostream>

CPU::CPU(Chip8& emulator) :
	_emulator(emulator),
	_memory(emulator.memory()),
	_display(emulator.display()),
	_input(emulator.input()),
	_pc(Chip8::ROM_START_ADDR),
	_I(0),
	_delayTimer(0),
	_soundTimer(0),
	_drawThisFrame(false)
{
	memset(_registers, 0, CPU::MAX_REGISTER);
	_memory.clear();
}

void CPU::initialize()
{
	// NNN: address
	// NN : 8 bit constant
	// N : 4 bit constant
	// X and Y : 4 bit register identifier
	// VN: 4 bit register. N may be 0x0 to 0xF
	// PC : Program Counter
	// I : 12bit register (For memory address)

	addInstruction(0x0000, 0x0FFF, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 0NNN: Unused
	});
	addInstruction(0xFFFF, 0x00E0, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 00E0: Clears the screen
		_display.clear();
	});
	addInstruction(0xFFFF, 0x00EE, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 00EE: Returns from a subroutine
		// Set pc to the the last address from the stack
		if (!_stack.empty())
		{
			_pc = _stack.top();
			_stack.pop();
		}
	});
	addInstruction(0xF000, 0x1000, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 1NNN: Jumps to address NNN
		_pc = opCode & 0x0FFF;
	});
	addInstruction(0xF000, 0x2000, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 2NNN: Calls subroutine at NNN
		_stack.push(_pc);
		_pc = opCode & 0x0FFF;
	});
	addInstruction(0xF000, 0x3000, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 3XNN: Skips the next instruction if VX equals NN
		if (_registers[b3] == (opCode & 0x00FF))
		{
			_pc += 2;
		}
	});
	addInstruction(0xF000, 0x4000, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 4XNN: Skips the next instruction if VX does not equal NN
		if (_registers[b3] != (opCode & 0x00FF))
		{
			_pc += 2;
		}
	});
	addInstruction(0xF00F, 0x5000, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 5XY0: Skips the next instruction if VX equals VY
		if (_registers[b3] == _registers[b2])
		{
			_pc += 2;
		}
	});
	addInstruction(0xF000, 0x6000, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 6XNN: Sets VX to NN
		_registers[b3] = opCode & 0x00FF;
	});
	addInstruction(0xF000, 0x7000, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 7XNN: Adds NN to VX
		_registers[b3] += opCode & 0x00FF;
	});
	addInstruction(0xF00F, 0x8000, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 8XY0: Sets VX to the value of VY
		_registers[b3] = _registers[b2];
	});
	addInstruction(0xF00F, 0x8001, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 8XY1: Sets VX to VX or VY
		_registers[b3] |= _registers[b2];
		if (_emulator.isVfResetEnabled())
		{
			_registers[0xF] = 0;
		}
	});
	addInstruction(0xF00F, 0x8002, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 8XY2: Sets VX to VX and VY
		_registers[b3] &= _registers[b2];
		if (_emulator.isVfResetEnabled())
		{
			_registers[0xF] = 0;
		}
	});
	addInstruction(0xF00F, 0x8003, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// BXY3: Sets VX to VX xor VY
		_registers[b3] ^= _registers[b2];
		if (_emulator.isVfResetEnabled())
		{
			_registers[0xF] = 0;
		}
	});
	addInstruction(0xF00F, 0x8004, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 8XY4: Adds VY to VX.
		// VF is set to 1 when there's an overflow, and to 0 when there is not
		bool isOverflow = (_registers[b3] + _registers[b2]) > 0xFF;
		_registers[b3] = _registers[b3] + _registers[b2];
		_registers[0xF] = isOverflow;
	});
	addInstruction(0xF00F, 0x8005, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 8XY5: VY is subtracted from VX
		// VF is set to 0 when there's an underflow, and 1 when there is not
		bool isOverflow = _registers[b3] >= _registers[b2];
		_registers[b3] = _registers[b3] - _registers[b2];
		_registers[0xF] = isOverflow;
	});
	addInstruction(0xF00F, 0x8006, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 8XY6: Shifts VX to the right by 1
		// Stores the least significant bit of VX prior to the shift into VF
		bool isOverflow = _registers[b3] & 0x01;
		if (_emulator.isShiftingEnabled())
		{
			_registers[b3] = _registers[b2];
		}
		_registers[b3] >>= 1;
		_registers[0xF] = isOverflow;
	});
	addInstruction(0xF00F, 0x8007, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 8XY7: Sets VX to VY minus VX
		// VF is set to 0 when there's an underflow, and 1 when there is not
		bool isOverflow = _registers[b2] >= _registers[b3];
		_registers[b3] = _registers[b2] - _registers[b3];
		_registers[0xF] = isOverflow;
	});
	addInstruction(0xF00F, 0x800E, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 8XYE: Shifts VX to the left by 1
		// Sets VF to 1 if the most significant bit of VX prior to that shift was set, or to 0 if it was unset
		bool isOverflow = (_registers[b3] & 0x80) >> 7;
		if (_emulator.isShiftingEnabled())
		{
			_registers[b3] = _registers[b2];
		}
		_registers[b3] <<= 1;
		_registers[0xF] = isOverflow;
	});
	addInstruction(0xF00F, 0x9000, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// 9XY0: Skips the next instruction if VX does not equal VY
		if (_registers[b3] != _registers[b2])
		{
			_pc += 2;
		}
	});
	addInstruction(0xF000, 0xA000, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// ANNN: Sets I to the address NNN
		_I = opCode & 0x0FFF;
	});
	addInstruction(0xF000, 0xB000, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// BNNN: Jumps to the address NNN plus V0
		_pc = _registers[0] + opCode & 0x0FFF;
	});
	addInstruction(0xF000, 0xC000, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// CXNN: Sets VX to the result of a bitwise and operation on a random number and NN.
		_registers[b3] = rand() & (opCode & 0x00FF);
	});
	addInstruction(0xF000, 0xD000, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
		// Each row of 8 pixels is read as bit-coded starting from memory location I
		// I value does not change after the execution of this instruction.
		// As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen.

		// Wrap if going beyond screen boundaries
		uint8_t startX = _registers[b3];
		uint8_t startY = _registers[b2];
		uint8_t height = b1;

		_registers[0xF] = 0;

		for (uint8_t y = 0; y < height; y++)
		{
			uint8_t spriteY = _memory.read8(_I + y);

			// Sprite are always 8 pixels wide
			for (uint8_t x = 0; x < 8; x++)
			{
				if (_emulator.isClippingEnabled())
				{
					// A sprite will be clipped if it’s partially drawn outside of display
					// but it will be wrapped around if all of the sprite is drawn outside of the display
					if ((startX < 64 && startX + x > 63) || (startY < 32 && startY + y > 31))
					{
						continue;
					}
				}

				uint8_t spritePixel = spriteY & (0x80 >> x);
				if (spritePixel)
				{
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
		_drawThisFrame = true;
	});
	addInstruction(0xF0FF, 0xE09E, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// EX9E: Skips the next instruction if the key stored in VX is pressed
		if (_input.isKeyDown(_registers[b3]))
		{
			_pc += 2;
		}
	});
	addInstruction(0xF0FF, 0xE0A1, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// EXA1: Skips the next instruction if the key stored in VX is not pressed
		if (!_input.isKeyDown(_registers[b3]))
		{
			_pc += 2;
		}
	});
	addInstruction(0xF0FF, 0xF007, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// FX07: Sets VX to the value of the delay timer
		_registers[b3] = _delayTimer;
	});
	addInstruction(0xF0FF, 0xF00A, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// FX0A: A key press is awaited, and then stored in VX (blocking operation, all instruction halted until next key event)

		bool isKeyPressed = false;
		for (uint8_t i = 0; i < Input::INPUT_COUNT; i++)
		{
			if (_input.getKeyState(i) == Input::KeyState::Released)
			{
				_registers[b3] = i;
				isKeyPressed = true;
				break;
			}
		}

		if (!isKeyPressed)
		{
			_pc -= 2;
		}
	});
	addInstruction(0xF0FF, 0xF015, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// FX15: Sets the delay timer to VX
		_delayTimer = _registers[b3];
	});
	addInstruction(0xF0FF, 0xF018, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// FX18: Sets the sound timer to VX
		_soundTimer = _registers[b3];
	});
	addInstruction(0xF0FF, 0xF01E, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// FX1E: Adds VX to I. VF is not affected
		_I += _registers[b3];
	});
	addInstruction(0xF0FF, 0xF029, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// FX29: Sets I to the location of the character in VX
		// Characters 0-F are represented by a 4x5 font
		_I = Chip8::FONT_START_ADDRESS + (_registers[b3] * 5);
	});
	addInstruction(0xF0FF, 0xF033, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// FX33: Stores the binary-coded decimal representation of VX in I:
		// - hundreds digit in memory at location in I,
		// - tens digit at location I+1
		// - ones digit at location I+2.
		_memory.write8(_I, (_registers[b3] / 100) % 10);
		_memory.write8(_I + 1, (_registers[b3] / 10) % 10);
		_memory.write8(_I + 2, _registers[b3] % 10);
	});
	addInstruction(0xF0FF, 0xF055, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// FX55: Stores from V0 to VX (including VX) in memory starting at address I
		// The offset from I is increased by 1 for each value written, but I itself is left unmodified
		for (uint8_t i = 0; i <= b3; i++)
		{
			_memory.write8(_I + i, _registers[i]);
		}

		if (_emulator.isSaveLoadIncrementEnabled())
		{
			_I += b3 + 1;
		}
	});
	addInstruction(0xF0FF, 0xF065, [&](uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1) {
		// FX65: Fills from V0 to VX (including VX) with values from memory, starting at address I
		// The offset from I is increased by 1 for each value read, but I itself is left unmodified
		for (uint8_t i = 0; i <= b3; i++)
		{
			_registers[i] = _memory.read8(_I + i);
		}

		if (_emulator.isSaveLoadIncrementEnabled())
		{
			_I += b3 + 1;
		}
	});
}

void CPU::addInstruction(uint16_t mask, uint16_t code, std::function<void(uint16_t opCode, uint8_t b3, uint8_t b2, uint8_t b1)> action)
{
	_instructions.emplace_back(CPU::Instruction(mask, code, action));
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
		// We provide bytes at position 1, 2, 3 for convenience, they are not always used
		uint8_t b3 = (opCode & (0x0F00)) >> 8;
		uint8_t b2 = (opCode & (0x00F0)) >> 4;
		uint8_t b1 = (opCode & (0x000F));
		instruction->action(opCode, b3, b2, b1);
	}
	else
	{
		std::cout << "[ERROR] Unknown opCode [" << std::hex << opCode << "]" << std::endl;
		return false;
	}

	return true;
}

void CPU::updateTimers()
{
	// This timer is intended to be used for timing the events of games.
	// If the timer value is zero, it stays zero otherwise it will decrement.
	if (_delayTimer > 0)
	{
		_delayTimer--;
	}

	// This timer is used for sound effects. When its value is nonzero, a beeping sound is made.
	// If the timer value is zero, it stays zero otherwise it will decrement.
	if (_soundTimer > 0)
	{
		_soundTimer--;
	}
}
