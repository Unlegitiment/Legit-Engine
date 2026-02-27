#pragma once
#include "ioMouse.h"
#include "ioKeyboard.h"
#include "ioGamepad.h"
namespace legit {
	class ioHandler {
	public:
		static void SetMousePrivate(ioMouse* pMouse, ioMouse::DeltaType DeltaX, ioMouse::DeltaType DeltaY, ioMouse::IOMouseType MouseX, ioMouse::IOMouseType MouseY);
		static void SetMousePrivate(ioMouse* pMouse, ioMouse::DeltaType DeltaX, ioMouse::DeltaType DeltaY, ioMouse::IOMouseType MouseX, ioMouse::IOMouseType MouseY, ioMouse::MouseNormalized NormalizedX, ioMouse::MouseNormalized NormalizedY);
		static void SetKey(ioKeyboard* pBoard, ioKey key, ioKeyState keyState);
		static ioKeyState* GetKeyStateFromBoard(ioKeyboard* pBoard);
		static ioMouse::DeltaType* GetMouseXDelta(ioMouse* pMouse);
		static ioMouse::DeltaType* GetMouseYDelta(ioMouse* pMouse);
		/*
			Returns pointer to array containing legit::ioGamePadButtons::MAX_GAMEPAD_BUTTONS size.
		*/
		static legit::ioGamePad::ButtonValue* GetControllerValues(legit::ioGamePad* pGamePad);
		static legit::ioGamePad::PressureValue* GetControllerPressure(legit::ioGamePad* pGamePad, legit::ioGamePadPressureInputs SelectedPressure);
		static legit::ioGamePad::RumbleValue* GetControllerRumbleRightMotor(legit::ioGamePad* pPad);
		static legit::ioGamePad::RumbleValue* GetControllerRumbleLeftMotor(legit::ioGamePad* pPad);

		static inline legit::ioGamePad::RumbleValue LastReportedMotorLValue = 0;
		static inline legit::ioGamePad::RumbleValue LastReportedMotorRValue = 0; 
	private:

	};
}