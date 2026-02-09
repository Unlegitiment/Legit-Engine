#pragma once
#include "../ioGamepad.h"
#include "../ioHandler.h"
#include <LITemplates/memutil/ZeroBlock.h>


#include <array>
#include <winerror.h>
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")


namespace legit {
	class XInput {
	public:
		struct sControllerInput {
			bool A = false, B = false, X = false, Y = false, LB = false, RB = false, RJoyButton = false, LJoyButton = false, Start = false, Back = false, DPadUp = false, DPadDown = false, DPadRight = false, DPadLeft = false;
			short LT = 0, RT = 0;
			short RightStickX = 0, LeftStickX = 0;
			short RightStickY = 0, LeftStickY = 0;
		};
	private:
		struct Controller {
			sControllerInput Input;
			bool IsConnected = false;
			unsigned long long ConnectedTime = 0llu;
		};
	public:
		static void Init() {
			IsXInputEnabled = true;
		}
		static void Update(legit::ioGamePad& io) {
			// poll incoming devices.
			for (int i = 0; i < XUSER_MAX_COUNT; i++) {
				XINPUT_STATE state;
				legit::ZeroBlock(&state);
				int Result = XInputGetState((DWORD)i, &state); // so this constantly polls for new controllers. 
				Controller& ctr = m_Controllers[i];
				if (Result == ERROR_SUCCESS) {
					ctr.IsConnected = true;
					auto* v = legit::ioHandler::GetControllerValues(&io);
					CaptureButtons(v, state);
					CapturePressures(&io, state);
					Vibrate(i, io);
				} else {
					ctr.IsConnected = false;
				}
			}
		}
		static void Shutdown() {

		}
		static void Vibrate(int UserIndex, legit::ioGamePad& ioGamePad) {
			auto* lmotor = legit::ioHandler::GetControllerRumbleLeftMotor(&ioGamePad);
			auto* rmotor = legit::ioHandler::GetControllerRumbleRightMotor(&ioGamePad); // this might spam gamepad 
			if (*lmotor == legit::ioHandler::LastReportedMotorLValue || *rmotor == legit::ioHandler::LastReportedMotorRValue) return; // we don't need to set a value that already exists.
			XINPUT_VIBRATION vib{};
			vib.wLeftMotorSpeed = *lmotor;
			vib.wRightMotorSpeed = *rmotor;
			XInputSetState(UserIndex, &vib);

			legit::ioHandler::LastReportedMotorLValue = *lmotor;
			legit::ioHandler::LastReportedMotorRValue = *rmotor;
		}
		static void Vibrate(int Index, unsigned short LeftMotor, unsigned short RightMotor) {
			XINPUT_VIBRATION vibration{};
			vibration.wLeftMotorSpeed = LeftMotor;
			vibration.wRightMotorSpeed = RightMotor;
			XInputSetState(Index, &vibration);
		}
		static void CapturePressures(legit::ioGamePad* pGamePad, XINPUT_STATE& state) {
			// probably the dumbest way to do this, but whatever im in the architecture until I can rewrite it.
			for (int i = 0; i < (int)legit::ioGamePadPressureInputs::MAX_PRESSURES; i++) {
				auto* ptr = legit::ioHandler::GetControllerPressure(pGamePad, (legit::ioGamePadPressureInputs)i);
				switch (i) {
					case (int)legit::ioGamePadPressureInputs::LeftStickX:
						*ptr = state.Gamepad.sThumbLX;
						break;
					case (int)legit::ioGamePadPressureInputs::LeftStickY:
						*ptr = state.Gamepad.sThumbLY;
						break;
					case (int)legit::ioGamePadPressureInputs::RightStickX:
						*ptr = state.Gamepad.sThumbRX;
						break;
					case (int)legit::ioGamePadPressureInputs::RightStickY:
						*ptr = state.Gamepad.sThumbRY;
						break;
					case (int)legit::ioGamePadPressureInputs::LT:
						*ptr = state.Gamepad.bLeftTrigger;
						break;
					case (int)legit::ioGamePadPressureInputs::RT:
						*ptr = state.Gamepad.bRightTrigger;
						break;
				}
			}
		}
		static void CaptureButtons(bool* ctr, XINPUT_STATE& state) {
			auto Buttons = state.Gamepad.wButtons;
			WORD b = state.Gamepad.wButtons;
			ctr[(int)legit::ioGamePadButtons::A] = b & XINPUT_GAMEPAD_A;
			ctr[(int)legit::ioGamePadButtons::B] = b & XINPUT_GAMEPAD_B;
			ctr[(int)legit::ioGamePadButtons::X] = b & XINPUT_GAMEPAD_X;
			ctr[(int)legit::ioGamePadButtons::Y] = b & XINPUT_GAMEPAD_Y;

			ctr[(int)legit::ioGamePadButtons::LB] = b & XINPUT_GAMEPAD_LEFT_SHOULDER;
			ctr[(int)legit::ioGamePadButtons::RB] = b & XINPUT_GAMEPAD_RIGHT_SHOULDER;

			ctr[(int)legit::ioGamePadButtons::Back] = b & XINPUT_GAMEPAD_BACK;
			ctr[(int)legit::ioGamePadButtons::Start] = b & XINPUT_GAMEPAD_START;

			ctr[(int)legit::ioGamePadButtons::LJoy] = b & XINPUT_GAMEPAD_LEFT_THUMB;
			ctr[(int)legit::ioGamePadButtons::RJoy] = b & XINPUT_GAMEPAD_RIGHT_THUMB;

			ctr[(int)legit::ioGamePadButtons::DPadUp] = b & XINPUT_GAMEPAD_DPAD_UP;
			ctr[(int)legit::ioGamePadButtons::DPadDown] = b & XINPUT_GAMEPAD_DPAD_DOWN;
			ctr[(int)legit::ioGamePadButtons::DPadLeft] = b & XINPUT_GAMEPAD_DPAD_LEFT;
			ctr[(int)legit::ioGamePadButtons::DPadRight] = b & XINPUT_GAMEPAD_DPAD_RIGHT;


		}
		static inline bool IsXInputEnabled = false;
		static inline std::array<Controller, XUSER_MAX_COUNT> m_Controllers;
	};
}
