#include "WindowsCmdArgs.h"
#include <Main/LIT_Start/ScopedPtr.h>
#include <Windows.h>

CWinArgs::CWinArgs(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t* pCmdLine, int nCmdShow)
	: m_HInstance(hInstance),
	m_hPrevInstance(hPrevInstance),
	m_pCmdLine(pCmdLine), // so these include all of our Cmd Args how do we parse args?
	m_nCmdShow(nCmdShow)
{
	int tempArgs = 0;
	ScopedPtr<LPWSTR> ArgList = ScopedPtr<LPWSTR>(CommandLineToArgvW(GetCommandLineW(), &tempArgs), [](LPWSTR* str)->void { LocalFree(str); });
	if (ArgList.Get() == NULL) {
		perror("[FATAL] Could not launch application, CommandLineToArgvW failed.");
		return;
	}
	m_iCmdArgs = tempArgs;
	m_CmdArgs.reserve(m_iCmdArgs);
	for (int i = 0; i < m_iCmdArgs; i++) {
		m_CmdArgs.push_back(ArgList.Get()[i]);
	}

}

HINSTANCE CWinArgs::GetHInstance() const { // if we are building for Win32
	return m_HInstance;
}

HINSTANCE CWinArgs::GetPrevHInstance() const {
	return m_hPrevInstance;
}

const wchar_t* CWinArgs::GetCmdLine() {
	return m_pCmdLine;
}

int CWinArgs::GetCmdShow() const {
	return this->m_nCmdShow;
}

const std::vector<std::wstring>& CWinArgs::GetCmdArgs()
{
	return m_CmdArgs;
}

int CWinArgs::GetNumCmdArgs()
{
	return m_iCmdArgs;
}
