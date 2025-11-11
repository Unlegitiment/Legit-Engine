#pragma once
#include <LECore/core/args.h>
#include <Windows.h>
class CWinArgs : public legit::fwCmdArgs {
public:
	CWinArgs(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t* pCmdLine, int nCmdShow);
	HINSTANCE GetHInstance() const;
	HINSTANCE GetPrevHInstance() const;
	const wchar_t* GetCmdLine();
	int GetCmdShow() const;
private:
	const std::vector<std::wstring>& GetCmdArgs() override;
	int GetNumCmdArgs() override;
	HINSTANCE m_HInstance, m_hPrevInstance;
	wchar_t* m_pCmdLine;
	int m_nCmdShow, m_iCmdArgs = 0;
	std::vector<std::wstring> m_CmdArgs;
};