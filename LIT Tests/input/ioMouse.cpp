#include "ioMouse.h"
using namespace legit;
ioMouse::DeltaType ioMouse::GetDeltaX() const {
	return this->MouseXDelta;
}

ioMouse::DeltaType ioMouse::GetDeltaY() const {
	return this->MouseYDelta;
}

ioMouse::IOMouseType ioMouse::GetX() const {
	return this->MouseX;
}

ioMouse::IOMouseType ioMouse::GetY() const {
	return this->MouseY;
}

ioMouse::MouseNormalized ioMouse::GetNormX() const {
	return MouseNormalizedX;
}

ioMouse::MouseNormalized ioMouse::GetNormY() const {
	return MouseNormalizedY;
}
