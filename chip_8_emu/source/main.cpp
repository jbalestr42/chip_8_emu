#include "Chip8.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Provide the rom as first argument." << std::endl;
		return 0;
	}

	Chip8 emulator(60, true, true, true, true, true);
	emulator.display().setPixelColorOff(sf::Color(35, 145, 157, 255));
	emulator.display().setPixelColorOn(sf::Color(180, 252, 252, 255));
	emulator.display().clear();

	if (emulator.loadRom(argv[1]))
	{
		emulator.initialize();
		emulator.update();
	}
	else
	{
		std::cout << "[ERROR] An error occured while loading the rom '" << argv[1] << "'" << std::endl;
	}

	return 0;
}
