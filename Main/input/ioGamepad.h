#pragma once
namespace legit {
	enum class ioGamePadButtons {
		A, B, X, Y, LB, RB, RJoy, LJoy, Start, Back, DPadUp, DPadDown, DPadRight, DPadLeft,
		MAX_GAMEPAD_BUTTONS
	};
	enum class ioGamePadPressureInputs {
		LT, RT, LeftStickX, RightStickX, LeftStickY, RightStickY,
		MAX_PRESSURES
	};
	class ioGamePad {
	public:
		friend class ioHandler;
		using ButtonValue = bool;
		using PressureValue = short;
		using PressureValueNormalized = float;
		using RumbleValue = unsigned short;
		ButtonValue GetButtonState(ioGamePadButtons button) const;
		PressureValue GetPressureValue(ioGamePadPressureInputs Input);
		PressureValueNormalized GetPressureValueNorm(ioGamePadPressureInputs PressureInputs);
		void Vibrate(RumbleValue MotorSpeed);
		/*
		This might not work because the RumbleValue is broken per-motor via XINPUT. Not too sure.
		*/
		void Vibrate(RumbleValue RightMotor, RumbleValue LeftMotor);
		bool IsVibrating() const;
	private:
		PressureValueNormalized GetMaximumForGamePadTrigger();
		PressureValueNormalized GetMaximumForGamePadStick();
		ButtonValue ButtonsState[(int)ioGamePadButtons::MAX_GAMEPAD_BUTTONS];
		PressureValue LT, RT;
		PressureValue RightStickX, RightStickY;
		PressureValue LeftStickX, LeftStickY;
		RumbleValue RightMotor = 0, LeftMotor = 0;
	};
}
