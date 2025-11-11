#pragma once
#include <LECore/core/args.h>
#include "WindowHandling/CWindow.h"
#include <iostream>
#include <LECore/maths/vec2.h>
#include <LECore/types.h>
class CApplication {
public:
	static CApplication& Get() {
		static CApplication m; return m;
	}
	void Init(legit::fwCmdArgs* args) {
		m_Arguments = args;
		return;
	}
	void Run() {
		while (!m_bStopRunning) {
			for (auto& a : m_Arguments->GetCmdArgs()) {
				wprintf(L"%s\n", a.c_str());
			}
			m_bStopRunning = true;
		}
		Shutdown();
	}
	void Shutdown() {

	}
private:
	legit::fwCmdArgs* m_Arguments = nullptr;
	bool m_bStopRunning = false;
};
class baseRawInputDevice {
public:
	virtual unsigned int GetFlags() = 0;
	virtual HWND GetTargetWindow() = 0;
	virtual unsigned short GetUsage() = 0;
	virtual unsigned short GetUsagePage() { return 0x01; }
	virtual unsigned long GetRIMType() = 0;
	virtual void OnInput(RAWINPUT* param) = 0;
	virtual RAWINPUTDEVICE GetDevice() {
		RAWINPUTDEVICE ret{};
		ret.usUsage = GetUsage();
		ret.usUsagePage = GetUsagePage();
		ret.hwndTarget = GetTargetWindow();
		ret.dwFlags = GetFlags();
		return ret;
	}
};


class CMouse {
public:
	static void InitClass() {
		sm_pMouse = new CMouse();
	}
	static CMouse* Get() {
		if (!sm_pMouse) {
			std::cout << "[CMouse] Mouse does not exist. You cannot use it here.";
			return nullptr;
		}
		return sm_pMouse;
	}
	static void Shutdown() {
		delete sm_pMouse;
	}
private:
	static inline CMouse* sm_pMouse = nullptr;
public:
	CMouse() {

	}
	void SetPosition(float fMouseX, float fMouseY) {
		this->m_Position.x = fMouseX;
		this->m_Position.y = fMouseY;
	}
	legit::Vector2f GetPosition() const { return m_Position; } 

	~CMouse() {

	}
private:
	legit::Vector2f m_Position;
};
class CRawInputMouse : public baseRawInputDevice {
public:
	CRawInputMouse() = default;
	CRawInputMouse(HWND wnd) :m_Window(wnd){

	}
	unsigned int GetFlags() override
	{
		return 0;
	}
	HWND GetTargetWindow() override
	{
		return m_Window;
	}
	unsigned short GetUsage() override
	{
		return 0x02;
	}
	unsigned long GetRIMType() {
		return RIM_TYPEMOUSE;
	}
	void OnInput(RAWINPUT* param) override
	{
		auto x = param->data.mouse.lLastX;
		auto y = param->data.mouse.lLastY;
		auto currentPos = CMouse::Get()->GetPosition();
		currentPos.x += x;
		currentPos.y += y;
		CMouse::Get()->SetPosition(currentPos.x, currentPos.y);
		std::cout << "[CRawMouse] Mouse Interacted with. lLastX: "<< currentPos.x  << " lLastY: " << currentPos.y << std::endl;
	}
private:
	HWND m_Window = NULL;
};

class baseUserInput {
public:
	virtual void Update(WPARAM wParam, LPARAM lParam) = 0;
private:

};
class CRawInput : public baseUserInput{
public:
	static constexpr int STD_IO_DRV = 0x01;
	static constexpr int MOUSE_ID = 0x02;
	static constexpr int KB_ID = 0x06;
	CRawInput() = default;
	CRawInput(const std::vector<baseRawInputDevice*>& devs) {
		this->m_Device = devs;
		Init();
	}
	std::vector<baseRawInputDevice*>& GetDevices() { return this->m_Device; }
	void Init() {
		if (m_Device.empty()) return; // there is no devices.
		RAWINPUTDEVICE* dev = new RAWINPUTDEVICE[m_Device.size()];
		for (int i = 0; i < m_Device.size(); i++) {
			auto& dat = m_Device[i];
			dev[i] = dat->GetDevice();
		}
		if (RegisterRawInputDevices(dev, m_Device.size(), sizeof(dev[0])) == FALSE) {
			std::cout << "[rawinput] failed to register input device: " << GetLastError() << std::endl;
		}
		delete[] dev;
	}
	void Update(WPARAM wParam, LPARAM lParam) {
		{
			HRESULT hResult{};
			UINT dwSize;

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
			LPBYTE lpb = new BYTE[dwSize]; // This is done consistently. Not very good. 

			if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
				OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

			RAWINPUT* raw = (RAWINPUT*)lpb;
			for (auto& dev : m_Device) {
				if (raw->header.dwType == dev->GetRIMType()) {
					dev->OnInput(raw);
				}
			}
			delete[] lpb;
			return;
		}
	}
	~CRawInput() {
		m_Device.clear();
	}
protected:

private:
	std::vector<baseRawInputDevice*> m_Device;
};
class baseRawInputProvider {
public:
	virtual std::vector<baseRawInputDevice*> GetInputDevices() = 0;
	virtual ~baseRawInputProvider() = default;
};
class CRawInputProviders : public baseRawInputProvider {
public:
	CRawInputProviders() {

	}
	CRawInputMouse* GetMouse() { return &m_Mouse; }
	std::vector<baseRawInputDevice*> GetInputDevices() {
		return { &m_Mouse };
	}
	~CRawInputProviders() {
	
	}
private:
	CRawInputMouse m_Mouse{};
};
class CControls {
public:
	enum eInputMode {
		RAW_INPUT,
		DIRECT_INPUT,
		WINDOWS_STD
	};
	static void Init() {
		CMouse::InitClass();
		sm_pInputProviders = new CRawInputProviders();
		m_Input = new CRawInput(sm_pInputProviders->GetInputDevices());
		SetMode(RAW_INPUT);
	}
	static void SetMode(eInputMode e) {
		m_CurrentUserInputMode = GetFromInputMode(e);
	}
	static void Update(WPARAM wParam, LPARAM lParam) {
		if (!m_CurrentUserInputMode) {
			std::cout << "[CControls] Application does not have a specified UserInputMode.\n\0";
			return;
		}
		m_CurrentUserInputMode->Update(wParam, lParam); 
	}
	static CRawInput* GetRawInput() {
		return m_Input;
	}
	static void Shutdown() {
		m_CurrentUserInputMode = nullptr;
		delete m_Input; m_Input = nullptr;
		delete sm_pInputProviders; sm_pInputProviders = nullptr;
		CMouse::Shutdown();
	}
private:
	static baseUserInput* GetFromInputMode(eInputMode mode) {
		switch (mode) {
		case RAW_INPUT:
			return m_Input;
		default:
			return nullptr;
		}
	}
	static inline CRawInputProviders* sm_pInputProviders = nullptr;
	static inline baseUserInput* m_CurrentUserInputMode = nullptr;
	static inline CRawInput* m_Input = nullptr;
};
struct WindowEventParams {
public:
	WPARAM wParam;
	LPARAM lParam;
};
class CWindowEvents {
public:
	static void InitClass() {
		sm_pInstance = new CWindowEvents();
	}
	static CWindowEvents* Get() { return sm_pInstance; }
	static void ShutdownClass() {
		delete sm_pInstance;
	}
	void EventToggle(legit::u32 Msg, WPARAM wParam, LPARAM lParam) {
		m_Params[Msg] = { wParam, lParam };
	}
	void EndFrame() {
		m_Params.clear(); 
	}
private:
	static inline CWindowEvents* sm_pInstance = nullptr;
	std::unordered_map<legit::u32, WindowEventParams> m_Params;
};
class CMainWindow : public CWindow {
public:
	CMainWindow(CWinArgs* args) :CWindow(args, L"lagWindow", L"GameWin32.exe", WindowProc) {
		ShowWindow(GetWindowHandle(), GetWindowArgs()->GetCmdShow());
		CWindowEvents::InitClass();
		CControls::Init();
	}

	static inline bool g_Close = false;
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		HRESULT hResult{};
		CWindowEvents::Get()->EventToggle(uMsg, wParam, lParam); // have to check this
		switch (uMsg) {
		case WM_DESTROY:
			g_Close = true;
			PostQuitMessage(0);
			break;
		case WM_INPUT:
			CControls::Update(wParam, lParam);
			break;
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	void Poll() {
		CWindow::Poll();
		CWindowEvents::Get()->EndFrame();
	}
	~CMainWindow() {
		CControls::Shutdown();
		CWindowEvents::ShutdownClass();
	}
private:
	static CMainWindow& Get() {
		return *m_pWindow;
	}
	static inline CMainWindow* m_pWindow = nullptr;
};