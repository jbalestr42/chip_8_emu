#include "Input.hpp"
#include <cstring>

Input::Input()
{
	_bindings[0] = sf::Keyboard::Key::Num0;
	_bindings[1] = sf::Keyboard::Key::Num1;
	_bindings[2] = sf::Keyboard::Key::Num2;
	_bindings[3] = sf::Keyboard::Key::Num3;
	_bindings[4] = sf::Keyboard::Key::Num4;
	_bindings[5] = sf::Keyboard::Key::Num5;
	_bindings[6] = sf::Keyboard::Key::Num6;
	_bindings[7] = sf::Keyboard::Key::Num7;
	_bindings[8] = sf::Keyboard::Key::Num8;
	_bindings[9] = sf::Keyboard::Key::Num9;
	_bindings[10] = sf::Keyboard::Key::A;
	_bindings[11] = sf::Keyboard::Key::B;
	_bindings[12] = sf::Keyboard::Key::C;
	_bindings[13] = sf::Keyboard::Key::D;
	_bindings[14] = sf::Keyboard::Key::E;
	_bindings[15] = sf::Keyboard::Key::F;
	
	memset(&_inputs, 0, Input::INPUT_COUNT);
}

void Input::tick()
{
	for (uint8_t i = 0; i < Input::INPUT_COUNT; i++)
	{
		if (sf::Keyboard::isKeyPressed(_bindings[i]))
		{
			// KeyState::Pressed is set only one frame
			// then it's set to KeyState::Down
			_inputs[i] = _inputs[i] == Input::KeyState::None ? Input::KeyState::Pressed : Input::KeyState::Down;
		}
		else
		{
			// KeyState::Released is set only one frame if it was previously KeyState::Pressed or KeyState::Down
			// then it's set to KeyState::None
			_inputs[i] = _inputs[i] == Input::KeyState::Pressed || _inputs[i] == Input::KeyState::Down ? Input::KeyState::Released : Input::KeyState::None;
		}
	}
}

bool Input::isKeyDown(uint8_t keyCode)
{
	return _inputs[keyCode] == Input::KeyState::Pressed || _inputs[keyCode] == Input::KeyState::Down;
}

Input::KeyState Input::getKeyState(uint8_t keyCode)
{
	return _inputs[keyCode];
}