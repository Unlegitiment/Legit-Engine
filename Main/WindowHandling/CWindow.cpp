#include "CWindow.h"


CWindow::CWindow(CWinArgs* Windows, const wchar_t* WindowClassName, const wchar_t* WindowName, WindowProcess m_cbProc) : m_pWindowArgs(Windows), m_Proc(m_cbProc) {
	WNDCLASS wc = {};
	wc.lpfnWndProc = m_Proc;
	wc.hInstance = m_pWindowArgs->GetHInstance();
	wc.lpszClassName = WindowClassName;
	RegisterClass(&wc);
	this->m_WndClass = wc;
	m_Hwnd = CreateWindowEx(0, WindowClassName, WindowName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, m_pWindowArgs->GetHInstance(), NULL);
	if (m_Hwnd == NULL) {
		return;
	}
}

void CWindow::Poll() {
	while (PeekMessage(&m_CurrentMsg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&m_CurrentMsg);
		DispatchMessage(&m_CurrentMsg);
	}
}

HWND CWindow::GetWindowHandle() const {
	return this->m_Hwnd;
}

const WNDCLASS& CWindow::GetWindowClass() { // Do not change this.
	return this->m_WndClass;
}

const CWinArgs* CWindow::GetWindowArgs() {
	return m_pWindowArgs;
}
