#pragma once
#include "ioGamepad.h"
#include "ioMouse.h"
#include "ioKeyboard.h"
#include "ioHandler.h"
#include "win/iobase.h"
#include "win/raw_input.h"
#include "win/xinput.h"
namespace legit {
	class ioInput {
	public:
		static void Init(void(*AddEventHandler)(WNDPROC)) {
			mMouse = new legit::ioMouse();
			mKeyboard = new legit::ioKeyboard();
			mGamePad = new legit::ioGamePad();
			CIOBasis::Init(AddEventHandler);
			CRawInput::Init(AddEventHandler, *mKeyboard, *mMouse);
			XInput::Init();
		}
		static legit::ioMouse& GetMouse() {
			return *mMouse;
		}
		static legit::ioKeyboard& GetKeyboard() {
			return *mKeyboard;
		}
		static legit::ioGamePad& GetGamePad() {
			return *mGamePad;
		}
		static void Update(float WindowWidth, float WindowHeight) {
			UpdateMouse(WindowHeight, WindowWidth);
			UpdateKeyboard();
			XInput::Update(*mGamePad);
		}
		static void Shutdown() {
			CRawInput::Shutdown();
			legit::XInput::Shutdown();
			legit::Delete(mGamePad);
			legit::Delete(mKeyboard);
			legit::Delete(mMouse);
		}
	private:
		static void UpdateMouse(float WindowHeight, float WindowWidth) {
			if (!mMouse) return;
			float x = float(CIOBasis::GetMouseX()) / WindowWidth;
			float y = float(CIOBasis::GetMouseY()) / WindowHeight;
			legit::ioHandler::SetMousePrivate(mMouse, CRawInput::MouseDeltaX(), CRawInput::MouseDeltaY(), CIOBasis::GetMouseX(), CIOBasis::GetMouseY(), x, y);
			CRawInput::ResetMouseDeltas(); // mouse deltas breaks functionality (as end frame is weird) 
		}
		static void UpdateKeyboard() {
			if (!mKeyboard)return;
			for (int i = 0; i < CRawInput::KEY_MAX; i++) {
				auto status = CRawInput::GetKeyStatus((legit::ioKey)i);
				legit::ioKeyState state{};
				state.bIsKeyDown = status;
				state.bIsKeyUp = !status; // kinda useless lmao.
				legit::ioHandler::SetKey(mKeyboard, (legit::ioKey)i, state);
			}
		}
		static inline legit::ioMouse* mMouse = nullptr;
		static inline legit::ioKeyboard* mKeyboard = nullptr;
		static inline legit::ioGamePad* mGamePad = nullptr;
	};
}