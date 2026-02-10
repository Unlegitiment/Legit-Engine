

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// god I fucking hate Windows.

#include "xInput.h"

#include <Xinput.h>
#pragma comment(lib, "xinput.lib")
using namespace legit;
void XInput::Init() {
	IsXInputEnabled = true;
}

void XInput::Update(ioGamePad& io) {
	// poll incoming devices.
	for (int i = 0; i < XUSER_MAX_COUNT; i++) {
		XINPUT_STATE state;
		ZeroBlock(&state);
		int Result = XInputGetState((DWORD)i, &state); // so this constantly polls for new controllers. 
		Controller& ctr = m_Controllers[i];
		if (!Result) {
			ctr.IsConnected = true;
			auto* v = ioHandler::GetControllerValues(&io);
			CaptureButtons(v, state);
			CapturePressures(&io, state);
			Vibrate(i, io);
		} else {
			ctr.IsConnected = false;
		}
	}
}

void XInput::Shutdown() {

}

void XInput::Vibrate(int UserIndex, ioGamePad& ioGamePad) {
	auto* lmotor = ioHandler::GetControllerRumbleLeftMotor(&ioGamePad);
	auto* rmotor = ioHandler::GetControllerRumbleRightMotor(&ioGamePad); // this might spam gamepad 
	if (*lmotor == ioHandler::LastReportedMotorLValue || *rmotor == ioHandler::LastReportedMotorRValue) return; // we don't need to set a value that already exists.
	XINPUT_VIBRATION vib{};
	vib.wLeftMotorSpeed = *lmotor;
	vib.wRightMotorSpeed = *rmotor;
	XInputSetState(UserIndex, &vib);

	ioHandler::LastReportedMotorLValue = *lmotor;
	ioHandler::LastReportedMotorRValue = *rmotor;
}

void XInput::Vibrate(int Index, unsigned short LeftMotor, unsigned short RightMotor) {
	XINPUT_VIBRATION vibration{};
	vibration.wLeftMotorSpeed = LeftMotor;
	vibration.wRightMotorSpeed = RightMotor;
	XInputSetState(Index, &vibration);
}

void XInput::CapturePressures(ioGamePad* pGamePad, XINPUT_STATE& state) {
	// probably the dumbest way to do this, but whatever im in the architecture until I can rewrite it.
	for (int i = 0; i < (int)ioGamePadPressureInputs::MAX_PRESSURES; i++) {
		auto* ptr = ioHandler::GetControllerPressure(pGamePad, (ioGamePadPressureInputs)i);
		switch (i) {
			case (int)ioGamePadPressureInputs::LeftStickX:
				*ptr = state.Gamepad.sThumbLX;
				break;
			case (int)ioGamePadPressureInputs::LeftStickY:
				*ptr = state.Gamepad.sThumbLY;
				break;
			case (int)ioGamePadPressureInputs::RightStickX:
				*ptr = state.Gamepad.sThumbRX;
				break;
			case (int)ioGamePadPressureInputs::RightStickY:
				*ptr = state.Gamepad.sThumbRY;
				break;
			case (int)ioGamePadPressureInputs::LT:
				*ptr = state.Gamepad.bLeftTrigger;
				break;
			case (int)ioGamePadPressureInputs::RT:
				*ptr = state.Gamepad.bRightTrigger;
				break;
		}
	}
}

void XInput::CaptureButtons(bool* ctr, XINPUT_STATE& state) {
	auto Buttons = state.Gamepad.wButtons;
	WORD b = state.Gamepad.wButtons;
	ctr[(int)ioGamePadButtons::A] = b & XINPUT_GAMEPAD_A;
	ctr[(int)ioGamePadButtons::B] = b & XINPUT_GAMEPAD_B;
	ctr[(int)ioGamePadButtons::X] = b & XINPUT_GAMEPAD_X;
	ctr[(int)ioGamePadButtons::Y] = b & XINPUT_GAMEPAD_Y;

	ctr[(int)ioGamePadButtons::LB] = b & XINPUT_GAMEPAD_LEFT_SHOULDER;
	ctr[(int)ioGamePadButtons::RB] = b & XINPUT_GAMEPAD_RIGHT_SHOULDER;

	ctr[(int)ioGamePadButtons::Back] = b & XINPUT_GAMEPAD_BACK;
	ctr[(int)ioGamePadButtons::Start] = b & XINPUT_GAMEPAD_START;

	ctr[(int)ioGamePadButtons::LJoy] = b & XINPUT_GAMEPAD_LEFT_THUMB;
	ctr[(int)ioGamePadButtons::RJoy] = b & XINPUT_GAMEPAD_RIGHT_THUMB;

	ctr[(int)ioGamePadButtons::DPadUp] = b & XINPUT_GAMEPAD_DPAD_UP;
	ctr[(int)ioGamePadButtons::DPadDown] = b & XINPUT_GAMEPAD_DPAD_DOWN;
	ctr[(int)ioGamePadButtons::DPadLeft] = b & XINPUT_GAMEPAD_DPAD_LEFT;
	ctr[(int)ioGamePadButtons::DPadRight] = b & XINPUT_GAMEPAD_DPAD_RIGHT;


}

XInput::sControllerInput& XInput::GetControllerViaIndex(int Index) {
	return m_Controllers[Index].Input;
}
