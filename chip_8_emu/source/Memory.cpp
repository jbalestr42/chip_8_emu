#include "Memory.hpp"
#include <cstring>

uint8_t Memory::read8(uint16_t addr)
{
	return _data[addr];
}

void Memory::write8(uint16_t addr, uint8_t value)
{
	_data[addr] = value;
}

void Memory::copyBuffer(uint16_t addr, uint8_t* buffer, size_t size)
{
	memcpy(&_data[addr], buffer, size);
}

void Memory::clear()
{
	memset(_data, 0, Memory::MEMORY_SIZE);
}
