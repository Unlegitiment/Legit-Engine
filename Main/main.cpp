#include <iostream>
#include <LECore/core/args.h>
#include <LECore/headers/platform_specs.h>
#include <string>
#include <Discord/Discord/Application.h>
#include "Application.h"
using namespace legit;

void CommonMain(fwCmdArgs* args) {
	CApplication::Get().Init(args);
	CApplication::Get().Run();
}

#ifdef LE_WIN
#include <Windows.h>
#include <d3d11.h>
void SetupConsole() {
	if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
		AllocConsole();
	}
	FILE* fout;
	FILE* ferr;
	freopen_s(&fout, "CONOUT$", "w", stdout);
	freopen_s(&ferr, "CONOUT$", "w", stderr);
	HANDLE hreadout;
	HANDLE hwriteout;

	HANDLE hreaderr;
	HANDLE hwriteerr;
	SECURITY_ATTRIBUTES sao = { sizeof(sao),NULL,TRUE };
	SECURITY_ATTRIBUTES sae = { sizeof(sae),NULL,TRUE };
	CreatePipe(&hreadout, &hwriteout, &sao, 0);
	CreatePipe(&hreaderr, &hwriteerr, &sae, 0);
	SetStdHandle(STD_OUTPUT_HANDLE, hwriteout);
	SetStdHandle(STD_ERROR_HANDLE, hwriteerr);

	std::wcout << std::flush;
	wprintf(L"\n");
	fflush(stdout);
}
#pragma comment(lib,"d3d11.lib")

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	SetupConsole();
	CWinArgs args{hInstance, hPrevInstance, pCmdLine, nCmdShow};
	CMainWindow m_Window = CMainWindow(&args);

	D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC Desc{};
	Desc.BufferDesc.Width = 0;
	Desc.BufferDesc.Height = 0;
	Desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	Desc.BufferDesc.RefreshRate.Numerator = 0;
	Desc.BufferDesc.RefreshRate.Denominator = 0;
	Desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	Desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	Desc.BufferCount = 1;
	Desc.OutputWindow = m_Window.GetWindowHandle();
	Desc.Windowed = true;
	Desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	Desc.Flags = 0;
	ID3D11Device* pDevice = nullptr;
	IDXGISwapChain* pSwapChain = nullptr;
	D3D_FEATURE_LEVEL ReturnLevel{};
	ID3D11DeviceContext* pContext = nullptr;
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG,
		&FeatureLevel,
		1,
		D3D11_SDK_VERSION,
		&Desc,
		&pSwapChain, &pDevice, &ReturnLevel, &pContext
	);
	if (hr != S_OK) {
		std::cout << "Failed to init D3D11, " << hr << std::endl;
		return 1;
	}
	ID3D11Buffer* m_Backbuffer = nullptr;
	hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&m_Backbuffer));
	if (hr != S_OK) {
		std::cout << "could not get bb" << hr;
		return 0;
	}
	ID3D11RenderTargetView* m_View{};
	hr = pDevice->CreateRenderTargetView(m_Backbuffer, nullptr, &m_View);
	if (hr != S_OK) {
		std::cout << "could not create rtv" << hr;
		return 0;
	}
	while (!CMainWindow::g_Close) {
		m_Window.Poll();
		float f[4] = { 0.4f, 0.0f, 0.0f, 0.0f }; // A doesn't appear to hold value.
		pContext->ClearRenderTargetView(m_View, f);
		pSwapChain->Present(1,0);
	}
	CommonMain(&args);
	pDevice->Release();
	pSwapChain->Release();
	pContext->Release();
	printf("\n");
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