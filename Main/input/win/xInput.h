#pragma once
#include "../ioGamepad.h"
#include "../ioHandler.h"
#include <LITemplates/memutil/ZeroBlock.h>
#include <array>

#include <Xinput.h>

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
		static constexpr unsigned char MAX_CTRL = 4;
		struct Controller {
			sControllerInput Input;
			bool IsConnected = false;
			unsigned long long ConnectedTime = 0llu;
		};
	public:
		static void Init();
		static void Update(legit::ioGamePad& io);
		static void Shutdown();
		static void Vibrate(int UserIndex, legit::ioGamePad& ioGamePad);
		static void Vibrate(int Index, unsigned short LeftMotor, unsigned short RightMotor);
		static void CapturePressures(legit::ioGamePad* pGamePad, XINPUT_STATE& state);
		static void CaptureButtons(bool* ctr, XINPUT_STATE& state);
		static sControllerInput& GetControllerViaIndex(int Index);
	private:
		static inline bool IsXInputEnabled = false;
		static inline std::array<Controller, MAX_CTRL> m_Controllers;
	};
}
