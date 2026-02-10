#pragma once
#include "../ioMouse.h"
#include <wtypes.h>
namespace legit{
	class CIOBasis {
	public:
		static void Init(void(*WindowHandlerAddition)(WNDPROC)); // @Todo, In later iterations this function pointer should be altered, to handle a different type. WNDPROC is tied explicitly to Windows. Thus making it a poor choice to base operations upon. Either pass a list of processors that gets extended or handle this else where, 
		static LRESULT WindowProc(HWND wnd, UINT m, WPARAM wParam, LPARAM lParam);
		static legit::ioMouse::IOMouseType GetMouseX();
		static legit::ioMouse::IOMouseType GetMouseY();
	private:
		static inline legit::ioMouse::IOMouseType MouseX = 0, MouseY = 0;
	};
}