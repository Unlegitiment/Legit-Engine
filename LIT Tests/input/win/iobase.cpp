#include "iobase.h"
#include <wtypes.h>
#include <windowsx.h>

void legit::CIOBasis::Init(void(*WindowHandlerAddition)(WNDPROC)) {
	WindowHandlerAddition(WindowProc);
}

LRESULT legit::CIOBasis::WindowProc(HWND wnd, UINT m, WPARAM wParam, LPARAM lParam) {
	switch (m) {
		case WM_MOUSEMOVE:
			MouseX = GET_X_LPARAM(lParam);
			MouseY = GET_Y_LPARAM(lParam);
			break;
	}
	return 0;
}

legit::ioMouse::IOMouseType legit::CIOBasis::GetMouseX() {
	return MouseX;
}

legit::ioMouse::IOMouseType legit::CIOBasis::GetMouseY() {
	return MouseY;
}
