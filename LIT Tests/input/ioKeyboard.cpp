#include "ioKeyboard.h"
using namespace legit;
ioKeyboard::ioKeyboard() {}

ioKeyState ioKeyboard::GetState(ioKey key) {
	return KeyState[(int)key];
}

bool ioKeyboard::IsDown(ioKey key) const {
	return KeyState[(int)key].bIsKeyDown;
}

bool ioKeyboard::IsUp(ioKey key) const {
	return KeyState[(int)key].bIsKeyUp; // really fucking useless, lmao. 
}
