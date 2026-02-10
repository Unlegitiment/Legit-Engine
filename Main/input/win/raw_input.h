#pragma once
#include "../ioKeyboard.h"
#include "../ioHandler.h"
#include "../ioMouse.h"
#include <wtypes.h>

namespace legit{
	class CRawInput {
	public:
		static bool IsNotAllowedToProcess(UINT msg, WPARAM wParam);
		static legit::ioKey GetFromScanCode(unsigned short scanCode, bool extended);
		static LRESULT WindowProc(HWND wnd, UINT m, WPARAM wParam, LPARAM lParam);
		static constexpr int KEY_MAX = (int)legit::ioKey::MAX_IO_KEY;
		static void Init(void(*AddToEventHandler)(WNDPROC), legit::ioKeyboard& kb, legit::ioMouse& mouse);
		static void SetKey(legit::ioKey ke, bool IsDown);
		static bool& GetKeyStatus(legit::ioKey ke);
		static bool IsKeyDown(legit::ioKey key);
		static bool IsKeyUp(legit::ioKey key);
		static void ResetMouseDeltas();
		static void Shutdown();
		static int MouseDeltaX();
		static int MouseDeltaY();
	private:
		static RAWINPUT GetBuffer(LPARAM lParam);
		static char GetWParamCode(WPARAM wpar);
		static inline int MouseXDelta = 0;
		static inline int MouseYDelta = 0;
		static inline legit::ioKeyState* KbdData{nullptr};
	};
}