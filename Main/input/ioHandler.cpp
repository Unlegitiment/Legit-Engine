#include "ioHandler.h"
using namespace legit;

void ioHandler::SetMousePrivate(ioMouse* pMouse, ioMouse::DeltaType DeltaX, ioMouse::DeltaType DeltaY, ioMouse::IOMouseType MouseX, ioMouse::IOMouseType MouseY) {
	SetMousePrivate(pMouse, DeltaX, DeltaY, MouseX, MouseY, 0, 0);
}

void ioHandler::SetMousePrivate(ioMouse* pMouse, ioMouse::DeltaType DeltaX, ioMouse::DeltaType DeltaY, ioMouse::IOMouseType MouseX, ioMouse::IOMouseType MouseY, ioMouse::MouseNormalized NormalizedX, ioMouse::MouseNormalized NormalizedY) {
	pMouse->MouseX = MouseX;
	pMouse->MouseY = MouseY;
	pMouse->MouseXDelta = DeltaX;
	pMouse->MouseYDelta = DeltaY;
	pMouse->MouseNormalizedX = NormalizedX;
	pMouse->MouseNormalizedY = NormalizedY;
}

void ioHandler::SetKey(ioKeyboard* pBoard, ioKey key, ioKeyState keyState) {
	pBoard->KeyState[(int)key] = keyState;
}

ioKeyState* ioHandler::GetKeyStateFromBoard(ioKeyboard* pBoard) {
	return pBoard->KeyState;
}

ioMouse::DeltaType* ioHandler::GetMouseXDelta(ioMouse* pMouse) {
	return &pMouse->MouseXDelta;
}

ioMouse::DeltaType* ioHandler::GetMouseYDelta(ioMouse* pMouse) {
	return &pMouse->MouseYDelta;
}

/*
Returns pointer to array containing ioGamePadButtons::MAX_GAMEPAD_BUTTONS size.
*/
ioGamePad::ButtonValue* ioHandler::GetControllerValues(ioGamePad* pGamePad) {
	return pGamePad->ButtonsState;
}

ioGamePad::PressureValue* ioHandler::GetControllerPressure(ioGamePad* pGamePad, ioGamePadPressureInputs SelectedPressure) {
	switch (SelectedPressure) {
		case ioGamePadPressureInputs::LeftStickX: return &pGamePad->LeftStickX;
		case ioGamePadPressureInputs::LeftStickY: return &pGamePad->LeftStickY;
		case ioGamePadPressureInputs::RightStickX: return &pGamePad->RightStickX;
		case ioGamePadPressureInputs::RightStickY: return &pGamePad->RightStickY;
		case ioGamePadPressureInputs::LT: return &pGamePad->LT;
		case ioGamePadPressureInputs::RT: return &pGamePad->RT;
		default: return nullptr;
	}
}

ioGamePad::RumbleValue* ioHandler::GetControllerRumbleRightMotor(ioGamePad* pPad) {
	return &pPad->RightMotor;
}

ioGamePad::RumbleValue* ioHandler::GetControllerRumbleLeftMotor(ioGamePad* pPad) {
	return &pPad->LeftMotor;
}
