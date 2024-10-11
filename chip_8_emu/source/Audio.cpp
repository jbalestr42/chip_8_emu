#include "Audio.hpp"

static sf::Int16 squareWave(int sample, int freq, double amp)
{
	int tpc = 44100 / freq; // ticks per cycle
	int cyclepart = sample % tpc;
	int halfcycle = tpc / 2;
	sf::Int16 amplitude = static_cast<sf::Int16>(32767 * amp);
	return cyclepart < halfcycle ? amplitude : 0;
}

Audio::Audio()
{
	unsigned int channelCount = 1;
	unsigned int sampleRate = 44100;
	std::vector<sf::Int16> samples;

	for (unsigned int i = 0; i < sampleRate; i++)
	{
		samples.push_back(squareWave(i, 440, 0.9));
	}

	_buffer.loadFromSamples(&samples[0], samples.size(), channelCount, sampleRate);
	_sound.setBuffer(_buffer);
	_sound.setLoop(true);
}

void Audio::playSound()
{
	if (_sound.getStatus() != sf::SoundSource::Status::Playing)
	{
		_sound.play();
	}
}

void Audio::stopSound()
{
	_sound.stop();
}
