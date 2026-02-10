#pragma once
#include "ioGamepad.h"
#include "ioMouse.h"
#include "ioKeyboard.h"
#include "ioHandler.h"
#include <wtypes.h>
namespace legit {
	class ioInput {
	public:
		static void Init(void(*AddEventHandler)(WNDPROC));
		static legit::ioMouse& GetMouse();
		static legit::ioKeyboard& GetKeyboard();
		static legit::ioGamePad& GetGamePad();
		static void Update(float WindowWidth, float WindowHeight);
		static void Shutdown();
	private:
		static void UpdateMouse(float WindowHeight, float WindowWidth);
		static void UpdateKeyboard();
		static inline legit::ioMouse* mMouse = nullptr;
		static inline legit::ioKeyboard* mKeyboard = nullptr;
		static inline legit::ioGamePad* mGamePad = nullptr;
	};
}