#include "ioInput.h"
#include "win/iobase.h"
#include "win/raw_input.h"
#include "win/xinput.h"
#include <LITemplates/alloc/Default.h>
using namespace legit;
void ioInput::Init(void(*AddEventHandler)(WNDPROC)) {
	mMouse = new ioMouse();
	mKeyboard = new ioKeyboard();
	mGamePad = new ioGamePad();
	CIOBasis::Init(AddEventHandler);
	CRawInput::Init(AddEventHandler, *mKeyboard, *mMouse);
	XInput::Init();
}

ioMouse& ioInput::GetMouse() {
	return *mMouse;
}

ioKeyboard& ioInput::GetKeyboard() {
	return *mKeyboard;
}

ioGamePad& ioInput::GetGamePad() {
	return *mGamePad;
}

void ioInput::Update(float WindowWidth, float WindowHeight) {
	UpdateMouse(WindowHeight, WindowWidth);
	UpdateKeyboard();
	XInput::Update(*mGamePad);
}

void ioInput::Shutdown() {
	CRawInput::Shutdown();
	XInput::Shutdown();
	Delete(mGamePad);
	Delete(mKeyboard);
	Delete(mMouse);
}

void ioInput::UpdateMouse(float WindowHeight, float WindowWidth) {
	if (!mMouse) return;
	float x = float(CIOBasis::GetMouseX()) / WindowWidth;
	float y = float(CIOBasis::GetMouseY()) / WindowHeight;
	ioHandler::SetMousePrivate(mMouse, CRawInput::MouseDeltaX(), CRawInput::MouseDeltaY(), CIOBasis::GetMouseX(), CIOBasis::GetMouseY(), x, y);
	CRawInput::ResetMouseDeltas(); // mouse deltas breaks functionality (as end frame is weird) 
}

void ioInput::UpdateKeyboard() {
	if (!mKeyboard)return;
	for (int i = 0; i < CRawInput::KEY_MAX; i++) {
		auto status = CRawInput::GetKeyStatus((ioKey)i);
		ioKeyState state{};
		state.bIsKeyDown = status;
		state.bIsKeyUp = !status; // kinda useless lmao.
		ioHandler::SetKey(mKeyboard, (ioKey)i, state);
	}
}
