#pragma once

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

class Audio
{
public:
	Audio();

	void playSound();
	void stopSound();

private:
	sf::SoundBuffer _buffer;
	sf::Sound _sound;
};
