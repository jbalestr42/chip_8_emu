#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <cstdint>

class Input
{
public:
	enum KeyState
	{
		None,
		Pressed,
		Down,
		Released
	};

	Input();

	void tick();
	bool isKeyDown(uint8_t keyCode);
	Input::KeyState getKeyState(uint8_t keyCode);

	static const uint8_t INPUT_COUNT = 16;

private:
	KeyState _inputs[INPUT_COUNT];
	sf::Keyboard::Key _bindings[INPUT_COUNT];
};
