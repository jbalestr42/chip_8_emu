#pragma once

#include <cstdint>

class Memory
{
public:
	uint8_t read8(uint16_t addr) const;
	void write8(uint16_t addr, uint8_t value);
	void copyBuffer(uint16_t addr, uint8_t* buffer, size_t size);
	void clear();

	static const uint16_t MEMORY_SIZE = 4096;

private:
	uint8_t _data[MEMORY_SIZE];
};
