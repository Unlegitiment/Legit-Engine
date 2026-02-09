#pragma once
#include "ioMouse.h"
#include "ioKeyboard.h"
#include "ioGamepad.h"
namespace legit {
	class ioHandler {
	public:
		static void SetMousePrivate(ioMouse* pMouse, ioMouse::DeltaType DeltaX, ioMouse::DeltaType DeltaY, ioMouse::IOMouseType MouseX, ioMouse::IOMouseType MouseY) {
			SetMousePrivate(pMouse, DeltaX, DeltaY, MouseX, MouseY, 0, 0);
		}
		static void SetMousePrivate(ioMouse* pMouse, ioMouse::DeltaType DeltaX, ioMouse::DeltaType DeltaY, ioMouse::IOMouseType MouseX, ioMouse::IOMouseType MouseY, ioMouse::MouseNormalized NormalizedX, ioMouse::MouseNormalized NormalizedY) {
			pMouse->MouseX = MouseX;
			pMouse->MouseY = MouseY;
			pMouse->MouseXDelta = DeltaX;
			pMouse->MouseYDelta = DeltaY;
			pMouse->MouseNormalizedX = NormalizedX;
			pMouse->MouseNormalizedY = NormalizedY;
		}
		static void SetKey(ioKeyboard* pBoard, ioKey key, ioKeyState keyState) {
			pBoard->KeyState[(int)key] = keyState;
		}
		static ioKeyState* GetKeyStateFromBoard(ioKeyboard* pBoard) {
			return pBoard->KeyState;
		}
		static ioMouse::DeltaType* GetMouseXDelta(ioMouse* pMouse) {
			return &pMouse->MouseXDelta;
		}
		static ioMouse::DeltaType* GetMouseYDelta(ioMouse* pMouse) {
			return &pMouse->MouseYDelta;
		}
		/*
			Returns pointer to array containing legit::ioGamePadButtons::MAX_GAMEPAD_BUTTONS size.
		*/
		static legit::ioGamePad::ButtonValue* GetControllerValues(legit::ioGamePad* pGamePad) {
			return pGamePad->ButtonsState;
		}
		static legit::ioGamePad::PressureValue* GetControllerPressure(legit::ioGamePad* pGamePad, legit::ioGamePadPressureInputs SelectedPressure) {
			switch (SelectedPressure) {
				case legit::ioGamePadPressureInputs::LeftStickX: return &pGamePad->LeftStickX;
				case legit::ioGamePadPressureInputs::LeftStickY: return &pGamePad->LeftStickY;
				case legit::ioGamePadPressureInputs::RightStickX: return &pGamePad->RightStickX;
				case legit::ioGamePadPressureInputs::RightStickY: return &pGamePad->RightStickY;
				case legit::ioGamePadPressureInputs::LT: return &pGamePad->LT;
				case legit::ioGamePadPressureInputs::RT: return &pGamePad->RT;
				default: return nullptr;
			}
		}
		static legit::ioGamePad::RumbleValue* GetControllerRumbleRightMotor(legit::ioGamePad* pPad) {
			return &pPad->RightMotor;
		}
		static legit::ioGamePad::RumbleValue* GetControllerRumbleLeftMotor(legit::ioGamePad* pPad) {
			return &pPad->LeftMotor;
		}

		static inline legit::ioGamePad::RumbleValue LastReportedMotorLValue = 0;
		static inline legit::ioGamePad::RumbleValue LastReportedMotorRValue = 0;
	private:

	};
}