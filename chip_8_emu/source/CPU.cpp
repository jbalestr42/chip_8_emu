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
	addInstruction(0x0000, 0x0FFF, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 0NNN: Unused
	});
	addInstruction(0xFFFF, 0x00E0, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 00E0: Clears the screen
		_display.clear();
	});
	addInstruction(0xFFFF, 0x00EE, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 00EE: Returns from a subroutine
		// Set pc to the the last address from the stack
		if (!_stack.empty())
		{
			_pc = _stack.top();
			_stack.pop();
		}
	});
	addInstruction(0xF000, 0x1000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 1NNN: Jumps to address NNN
		_pc = NNN;
	});
	addInstruction(0xF000, 0x2000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 2NNN: Calls subroutine at NNN
		_stack.push(_pc);
		_pc = NNN;
	});
	addInstruction(0xF000, 0x3000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 3XNN: Skips the next instruction if VX equals NN
		if (_registers[X] == NN)
		{
			_pc += 2;
		}
	});
	addInstruction(0xF000, 0x4000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 4XNN: Skips the next instruction if VX does not equal NN
		if (_registers[X] != NN)
		{
			_pc += 2;
		}
	});
	addInstruction(0xF00F, 0x5000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 5XY0: Skips the next instruction if VX equals VY
		if (_registers[X] == _registers[Y])
		{
			_pc += 2;
		}
	});
	addInstruction(0xF000, 0x6000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 6XNN: Sets VX to NN
		_registers[X] = NN;
	});
	addInstruction(0xF000, 0x7000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 7XNN: Adds NN to VX
		_registers[X] += NN;
	});
	addInstruction(0xF00F, 0x8000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 8XY0: Sets VX to the value of VY
		_registers[X] = _registers[Y];
	});
	addInstruction(0xF00F, 0x8001, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 8XY1: Sets VX to VX or VY
		_registers[X] |= _registers[Y];
		if (_emulator.isVfResetEnabled())
		{
			_registers[0xF] = 0;
		}
	});
	addInstruction(0xF00F, 0x8002, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 8XY2: Sets VX to VX and VY
		_registers[X] &= _registers[Y];
		if (_emulator.isVfResetEnabled())
		{
			_registers[0xF] = 0;
		}
	});
	addInstruction(0xF00F, 0x8003, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// BXY3: Sets VX to VX xor VY
		_registers[X] ^= _registers[Y];
		if (_emulator.isVfResetEnabled())
		{
			_registers[0xF] = 0;
		}
	});
	addInstruction(0xF00F, 0x8004, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 8XY4: Adds VY to VX.
		// VF is set to 1 when there's an overflow, and to 0 when there is not
		bool isOverflow = (_registers[X] + _registers[Y]) > 0xFF;
		_registers[X] = _registers[X] + _registers[Y];
		_registers[0xF] = isOverflow;
	});
	addInstruction(0xF00F, 0x8005, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 8XY5: VY is subtracted from VX
		// VF is set to 0 when there's an underflow, and 1 when there is not
		bool isOverflow = _registers[X] >= _registers[Y];
		_registers[X] = _registers[X] - _registers[Y];
		_registers[0xF] = isOverflow;
	});
	addInstruction(0xF00F, 0x8006, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 8XY6: Shifts VX to the right by 1
		// Stores the least significant bit of VX prior to the shift into VF
		bool isOverflow = _registers[X] & 0x01;
		if (_emulator.isShiftingEnabled())
		{
			_registers[X] = _registers[Y];
		}
		_registers[X] >>= 1;
		_registers[0xF] = isOverflow;
	});
	addInstruction(0xF00F, 0x8007, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 8XY7: Sets VX to VY minus VX
		// VF is set to 0 when there's an underflow, and 1 when there is not
		bool isOverflow = _registers[Y] >= _registers[X];
		_registers[X] = _registers[Y] - _registers[X];
		_registers[0xF] = isOverflow;
	});
	addInstruction(0xF00F, 0x800E, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 8XYE: Shifts VX to the left by 1
		// Sets VF to 1 if the most significant bit of VX prior to that shift was set, or to 0 if it was unset
		bool isOverflow = (_registers[X] & 0x80) >> 7;
		if (_emulator.isShiftingEnabled())
		{
			_registers[X] = _registers[Y];
		}
		_registers[X] <<= 1;
		_registers[0xF] = isOverflow;
	});
	addInstruction(0xF00F, 0x9000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// 9XY0: Skips the next instruction if VX does not equal VY
		if (_registers[X] != _registers[Y])
		{
			_pc += 2;
		}
	});
	addInstruction(0xF000, 0xA000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// ANNN: Sets I to the address NNN
		_I = NNN;
	});
	addInstruction(0xF000, 0xB000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// BNNN: Jumps to the address NNN plus V0
		_pc = _registers[0] + NNN;
	});
	addInstruction(0xF000, 0xC000, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// CXNN: Sets VX to the result of a bitwise and operation on a random number and NN
		_registers[X] = rand() & NN;
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
			for (uint8_t x = 0; x < Chip8::SPRITE_WIDTH; x++)
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
	addInstruction(0xF0FF, 0xE09E, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// EX9E: Skips the next instruction if the key stored in VX is pressed
		if (_input.isKeyDown(_registers[X]))
		{
			_pc += 2;
		}
	});
	addInstruction(0xF0FF, 0xE0A1, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// EXA1: Skips the next instruction if the key stored in VX is not pressed
		if (!_input.isKeyDown(_registers[X]))
		{
			_pc += 2;
		}
	});
	addInstruction(0xF0FF, 0xF007, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// FX07: Sets VX to the value of the delay timer
		_registers[X] = _delayTimer;
	});
	addInstruction(0xF0FF, 0xF00A, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// FX0A: A key press is awaited, and then stored in VX (blocking operation, all instruction halted until next key event)

		bool isKeyPressed = false;
		for (uint8_t i = 0; i < Input::INPUT_COUNT; i++)
		{
			if (_input.getKeyState(i) == Input::KeyState::Released)
			{
				_registers[X] = i;
				isKeyPressed = true;
				break;
			}
		}

		if (!isKeyPressed)
		{
			_pc -= 2;
		}
	});
	addInstruction(0xF0FF, 0xF015, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// FX15: Sets the delay timer to VX
		_delayTimer = _registers[X];
	});
	addInstruction(0xF0FF, 0xF018, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// FX18: Sets the sound timer to VX
		_soundTimer = _registers[X];
	});
	addInstruction(0xF0FF, 0xF01E, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// FX1E: Adds VX to I. VF is not affected
		_I += _registers[X];
	});
	addInstruction(0xF0FF, 0xF029, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// FX29: Sets I to the location of the character in VX
		// Characters 0-F are represented by a 4x5 font
		_I = Chip8::FONT_START_ADDRESS + (_registers[X] * 5);
	});
	addInstruction(0xF0FF, 0xF033, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// FX33: Stores the binary-coded decimal representation of VX in I:
		// - hundreds digit in memory at location in I,
		// - tens digit at location I+1
		// - ones digit at location I+2.
		_memory.write8(_I, (_registers[X] / 100) % 10);
		_memory.write8(_I + 1, (_registers[X] / 10) % 10);
		_memory.write8(_I + 2, _registers[X] % 10);
	});
	addInstruction(0xF0FF, 0xF055, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// FX55: Stores from V0 to VX (including VX) in memory starting at address I
		// The offset from I is increased by 1 for each value written, but I itself is left unmodified
		for (uint8_t i = 0; i <= X; i++)
		{
			_memory.write8(_I + i, _registers[i]);
		}

		if (_emulator.isSaveLoadIncrementEnabled())
		{
			_I += X + 1;
		}
	});
	addInstruction(0xF0FF, 0xF065, [&](uint16_t NNN, uint8_t NN, uint8_t N, uint8_t X, uint8_t Y) {
		// FX65: Fills from V0 to VX (including VX) with values from memory, starting at address I
		// The offset from I is increased by 1 for each value read, but I itself is left unmodified
		for (uint8_t i = 0; i <= X; i++)
		{
			_registers[i] = _memory.read8(_I + i);
		}

		if (_emulator.isSaveLoadIncrementEnabled())
		{
			_I += X + 1;
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
