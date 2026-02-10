#include "ioGamepad.h"
using namespace legit;
ioGamePad::ButtonValue ioGamePad::GetButtonState(ioGamePadButtons button) const {
	return ButtonsState[(int)button];
}

ioGamePad::PressureValue ioGamePad::GetPressureValue(ioGamePadPressureInputs Input) {
	switch (Input) {
		case ioGamePadPressureInputs::LT: return LT;
		case ioGamePadPressureInputs::RT: return RT;
		case ioGamePadPressureInputs::LeftStickX: return LeftStickX;
		case ioGamePadPressureInputs::LeftStickY: return LeftStickY;
		case ioGamePadPressureInputs::RightStickY: return RightStickY;
		case ioGamePadPressureInputs::RightStickX: return RightStickX;
		default: return 0;
	}
}

ioGamePad::PressureValueNormalized ioGamePad::GetPressureValueNorm(ioGamePadPressureInputs PressureInputs) {
	switch (PressureInputs) {
		case ioGamePadPressureInputs::LT: return (PressureValueNormalized)LT / GetMaximumForGamePadTrigger();
		case ioGamePadPressureInputs::RT: return (PressureValueNormalized)RT / GetMaximumForGamePadTrigger();
		case ioGamePadPressureInputs::LeftStickX: return (PressureValueNormalized)LeftStickX / GetMaximumForGamePadStick();
		case ioGamePadPressureInputs::LeftStickY: return (PressureValueNormalized)LeftStickY / GetMaximumForGamePadStick();
		case ioGamePadPressureInputs::RightStickY: return (PressureValueNormalized)RightStickY / GetMaximumForGamePadStick();
		case ioGamePadPressureInputs::RightStickX: return (PressureValueNormalized)RightStickX / GetMaximumForGamePadStick();
		default: return 0;
	}
}

void ioGamePad::Vibrate(RumbleValue MotorSpeed) {
	this->LeftMotor = MotorSpeed;
	this->RightMotor = MotorSpeed;
}

/*
This might not work because the RumbleValue is broken per-motor via XINPUT. Not too sure.
*/
void ioGamePad::Vibrate(RumbleValue RightMotor, RumbleValue LeftMotor) {
	this->LeftMotor = LeftMotor;
	this->RightMotor = RightMotor;
}

bool ioGamePad::IsVibrating() const {
	return this->LeftMotor != 0 && this->RightMotor != 0;
}

ioGamePad::PressureValueNormalized ioGamePad::GetMaximumForGamePadTrigger() {
	return 255.f;
}

ioGamePad::PressureValueNormalized ioGamePad::GetMaximumForGamePadStick() {
	return 32767.0f;
}