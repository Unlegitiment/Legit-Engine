#pragma once
#include "WindowsCmdArgs.h"
#include <Windows.h>
using WindowProcess = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);

class CWindow {
public:
	CWindow(CWinArgs* Windows, const wchar_t* WindowClassName, const wchar_t* WindowName, WindowProcess m_cbProc);
	void Poll();
	HWND GetWindowHandle() const;
	const WNDCLASS& GetWindowClass();
	const CWinArgs* GetWindowArgs();
private:
	WindowProcess m_Proc;
	MSG m_CurrentMsg;
	HWND m_Hwnd;
	WNDCLASS m_WndClass;
	CWinArgs* m_pWindowArgs;
};

