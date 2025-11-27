#include <iostream>
#include <LECore/core/args.h>
#include <LECore/headers/platform_specs.h>
#include <string>
#include <Discord/Discord/Application.h>
#include "Application.h"
using namespace legit;

static void ArgsInit(fwCmdArgs* args) {
	legit::g_Args = args;
}

static void CommonMain() {
	CApplication::Get().Run();
}

#ifdef LE_WIN
#include <Windows.h>
#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	CWinArgs* args = new CWinArgs{hInstance, hPrevInstance, pCmdLine, nCmdShow};
	ArgsInit(args);
	CommonMain();
	delete args;
	return 0;
}
#else
class CArguments : public fwCmdArgs {
public:
	CArguments(int Argc, char** Argv) : m_iNumberOfArgs(Argc) {
		m_ActiveArguments.reserve(Argc);
		for (int i = 0; i < Argc; i++) {
			m_ActiveArguments.push_back(Argv[i]);
		}
	}
	const std::vector<std::wstring>& GetCmdArgs() {
		return this->m_ActiveArguments;
	}
	const int GetNumCmdArgs() {
		return this->m_iNumberOfArgs;
	}
private:
	int m_iNumberOfArgs;
	std::vector<std::wstring> m_ActiveArguments; // might want to copy these strings. not entirely sure. 
};
int main(int argc, char* argv[]) {
	CArguments args{argc, argv};
	CommonMain(args);
	return 0;
}
#endif