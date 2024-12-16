#include "Chip8.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Provide the rom path as first argument." << std::endl;
		return 0;
	}

	Chip8 emulator(8, 60);
	if (emulator.loadRom(argv[1]))
	{
		emulator.update();
	}
	else
	{
		std::cout << "[ERROR] An error occured while loading the rom '" << argv[1] << "'" << std::endl;
	}

	return 0;
}
