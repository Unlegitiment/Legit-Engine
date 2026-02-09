#pragma once
#include "../ioMouse.h"
#include <wtypes.h>
namespace legit{
	class CIOBasis {
	public:
		static void Init(void(*WindowHandlerAddition)(WNDPROC));
		static LRESULT WindowProc(HWND wnd, UINT m, WPARAM wParam, LPARAM lParam);
		static legit::ioMouse::IOMouseType GetMouseX();
		static legit::ioMouse::IOMouseType GetMouseY();
	private:
		static inline legit::ioMouse::IOMouseType MouseX = 0, MouseY = 0;
	};
}