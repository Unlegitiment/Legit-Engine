#pragma once
#define NOMINMAX
#include <LECore/core/args.h>
#include "WindowHandling/CWindow.h"
#include <iostream>
#include <LECore/maths/vec2.h>
#include <LECore/types.h>
#include <unordered_map>
#include "Logger/GameLogger.h"

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
		//std::cout << "[CRawMouse] Mouse Interacted with. lLastX: "<< currentPos.x  << " lLastY: " << currentPos.y << std::endl;
	}
private:
	HWND m_Window = NULL;
};
class CRawInput{
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
			UINT dwSize{};

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
class CRawInputProviders {
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
struct WindowEventParams {
public:
	WPARAM wParam;
	LPARAM lParam;
};
class CWindowEvents {
public:
	using WindowsCallbackSignature = void(*)(const WindowEventParams&);
	static void InitClass() {
		sm_pInstance = new CWindowEvents();
	}
	static CWindowEvents* Get() { 
		if (!sm_pInstance) {
			std::cout << "[CWindowEvents] InitClass has not been called yet." << std::endl;
			return nullptr;
		}
		return sm_pInstance; 
	}
	static void ShutdownClass() {
		delete sm_pInstance;
	}
	void EventToggle(legit::u32 Msg, WPARAM wParam, LPARAM lParam) {
		m_Params[Msg] = { wParam, lParam };
		FireEvent(Msg);
	}
	void Subscribe(legit::u32 uMsg, const WindowsCallbackSignature& sig) {
		this->MsgToCb[uMsg].push_back(sig);
	}
	void FireEvent(legit::u32 uMsg) {
		if (uMsg == WM_DESTROY) {
			PostQuitMessage(0);
			return;
		}
		if (!WasEventFiredOnRecentTick(uMsg)) return;
		auto res = GetFromEvent(uMsg);
		if (IsNullEvent(res)) return;
		for (auto& [evt, cbList] : this->MsgToCb) {
			//std::cout << "[WindowEvent::Event] : evt " << evt << "[Vector] : " << cbList.size() << " Mem: {\n" ;
			/*for (int i = 0; i < cbList.size(); i++) {
				std::cout << cbList.data()[i] << ",\n";
			}*/
			//std::cout << "}\n\0";
			if (evt == uMsg) {
				for (auto& cb : cbList) {
					//std::cout << "[WindowEvent::CallBackList]: " << cbList.size() << std::endl; // this is weird. When this is active it only calls it once but when it isn't it triple calls the CB?
					cb(res);
				}
				if (evt == WM_DESTROY) {
					//std::cout << "what the fuck bluddydiddy" << std::endl;
				}
			}
		}
	}
	bool WasEventFiredOnRecentTick(legit::u32 uMsg) {
		auto res = this->m_Params.find(uMsg);
		if (res == m_Params.end()) return false;
		return true;
	}
	const WindowEventParams& GetFromEvent(legit::u32 uMsg) {
		if (WasEventFiredOnRecentTick(uMsg)) {
			return m_Params[uMsg];
		}
		else {
			return nullEvent;
		}
	}
	bool IsNullEvent(const WindowEventParams& evtPrm) {
		return (evtPrm.lParam == NULL && evtPrm.wParam == NULL) ? true : false;
	}
	void EndFrame() {
		m_Params.clear();
	}
private:
	static inline CWindowEvents* sm_pInstance = nullptr;
	WindowEventParams nullEvent = { NULL, NULL };
	std::unordered_map<legit::u32, std::vector<WindowsCallbackSignature>> MsgToCb;
	std::unordered_map<legit::u32, WindowEventParams> m_Params;
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
		CWindowEvents::Get()->Subscribe(WM_INPUT, [](const WindowEventParams& evt)->void {
			CControls::WM_EVENT_LISTENER(evt.wParam, evt.lParam);
			//std::cout << "using callback" << std::endl;
			});
	}
	static void WM_EVENT_LISTENER(WPARAM wParam, LPARAM lParam) {
		GetRawInput()->Update(wParam, lParam);
	}
	static CRawInput* GetRawInput() {
		return m_Input;
	}
	static void Shutdown() {
		delete m_Input; m_Input = nullptr;
		delete sm_pInputProviders; sm_pInputProviders = nullptr;
		CMouse::Shutdown();
	}
private:
	static inline CRawInputProviders* sm_pInputProviders = nullptr;
	static inline CRawInput* m_Input = nullptr;
};
class CMainWindow{
public:
	CMainWindow(CWinArgs* args) : m_Window(args, L"lagWindow", L"GameWin32.exe", WindowProc) {
		m_pWindow = this;
		ShowWindow(m_Window.GetWindowHandle(), m_Window.GetWindowArgs()->GetCmdShow());
		CControls::Init();
	}
	bool g_Close = false;
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		HRESULT hResult{};
		if (uMsg == WM_DESTROY) {
			CMainWindow::Get().g_Close = true;
		}
		CWindowEvents::Get()->EventToggle(uMsg, wParam, lParam); // have to check this
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	void Poll() {
		m_Window.Poll();
		CWindowEvents::Get()->EndFrame();
	}
	~CMainWindow() {
		CloseWindow(m_Window.GetWindowHandle());
		CControls::Shutdown();
		CWindowEvents::ShutdownClass(); // check.
	}
	static CMainWindow& Get() {
		return *m_pWindow;
	}
	CWindow* GetRawWindow() { return &m_Window; }
private:
	static inline CMainWindow* m_pWindow = nullptr;
	CWindow m_Window;
};


#include <d3d11.h>

struct sOutDevice {
public:
	static sOutDevice* Init(void* Handle) {
		sOutDevice* device = new sOutDevice();
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
		Desc.OutputWindow = (HWND)Handle;
		Desc.Windowed = true;
		Desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		Desc.Flags = 0;
		D3D_FEATURE_LEVEL ReturnLevel{};
		HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			D3D11_CREATE_DEVICE_DEBUG,
			&FeatureLevel,
			1,
			D3D11_SDK_VERSION,
			&Desc,
			&device->m_pSwapChain, &device->m_pDevice, &ReturnLevel, &device->m_pContext
		);
		if (hr != S_OK) {
			std::cout << "Failed to init D3D11, " << hr << std::endl;
			return nullptr;
		}
		return device;
	}
	IDXGISwapChain* m_pSwapChain = nullptr;
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pContext = nullptr;
};
class CApplication;


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
//If blobs are not useful past the compile stage and the Input Assembler might scrap the in-class definition for compiling shaders and put it into a different object. 
class lagVertexShader {
public:
	static inline ID3D11Device* sm_Device = nullptr; // because this is so piss poor, i need to globalize the device. 
	static constexpr const char* SHADER_VER = "vs_5_0";
	static constexpr const char* SHADER_DBG_FAIL = "[VS] ";
	lagVertexShader(const wchar_t* path, const char* entryPoint, UINT flags) {
		ID3DBlob* ErrorBlob{};
		HRESULT hr;
		hr = D3DCompileFromFile(path, NULL, NULL, entryPoint, SHADER_VER, flags, 0, &m_Blob, &ErrorBlob); // This might be just static behavior. 
		if (FAILED(hr)) {
			std::cout << SHADER_DBG_FAIL << ErrorBlob->GetBufferPointer() << std::endl; 
			ErrorBlob->Release(); // Contain.
			return;
		}
		hr = sm_Device->CreateVertexShader(m_Blob->GetBufferPointer(), m_Blob->GetBufferSize(), nullptr, &m_ShaderProgram);
		if (FAILED(hr)) {
			std::cout << SHADER_DBG_FAIL << hr << std::endl;
			return;
		}
	}
	ID3D11VertexShader* GetShader() {
		return this->m_ShaderProgram;
	}
	ID3DBlob* GetBlob() {
		return this->m_Blob;
	}
	~lagVertexShader() {
		m_Blob->Release();
		m_ShaderProgram->Release(); 
	}
private:
	ID3DBlob* m_Blob{};
	ID3D11VertexShader* m_ShaderProgram;
};
class lagFragmentShader {
public:
	static inline ID3D11Device* sm_Device = nullptr;
	static constexpr const char* SHADER_VER = "ps_5_0";
	static constexpr const char* SHADER_DBG_FAIL = "[PS] ";
	lagFragmentShader(const wchar_t* path, const char* entryPoint, UINT flags) {
#pragma region SHADER_COMPILE
		ID3DBlob* ErrorBlob{};
		HRESULT hr;
		hr = D3DCompileFromFile(path, NULL, NULL, entryPoint, SHADER_VER, flags, 0, &m_Blob, &ErrorBlob); // This might be just static behavior. 
		if (FAILED(hr)) {
			std::cout << SHADER_DBG_FAIL << ErrorBlob->GetBufferPointer() << std::endl;
			ErrorBlob->Release(); // Contain.
			return;
		}
#pragma endregion 
		hr = sm_Device->CreatePixelShader(m_Blob->GetBufferPointer(), m_Blob->GetBufferSize(), nullptr, &m_ShaderProgram);
		if (FAILED(hr)) {
			std::cout << SHADER_DBG_FAIL << hr << std::endl;
			return;
		}
	}
	ID3D11PixelShader* GetShader() {
		return this->m_ShaderProgram;
	}
	ID3DBlob* GetBlob() {
		return this->m_Blob;
	}
	~lagFragmentShader() {
		m_Blob->Release();
		m_ShaderProgram->Release();
	}
private:
	ID3DBlob* m_Blob{};
	ID3D11PixelShader* m_ShaderProgram;
};
struct Vertex {
	DirectX::XMFLOAT3 m_Positions;
	DirectX::XMFLOAT3 m_Color;
};
class lagVertexBuffer {
public:
	static inline ID3D11Device* sm_Device = nullptr;
	lagVertexBuffer() {
		D3D11_BUFFER_DESC Description{};
		Description.ByteWidth = sizeof(vertices); // This is this size of my buffer?
		Description.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		Description.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
		Description.CPUAccessFlags = 0;
		Description.StructureByteStride = 0; // Im not sure how to use this?
		D3D11_SUBRESOURCE_DATA mData{};
		mData.pSysMem = this->vertices;
		HRESULT hr;
		hr = sm_Device->CreateBuffer(&Description, &mData, &m_VertexBuffer);

	}
	// obviously we don't want this here. This has nothing to do with the vertex buffer itself. this is just the separation phase of the shit.
	void Bind(ID3D11DeviceContext* Context) const {
		UINT stride = sizeof(Vertex);
		UINT offsets = 0;
		Context->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offsets);
	}
	~lagVertexBuffer() {
		m_VertexBuffer->Release(); 
	}
private:
	ID3D11Buffer* m_VertexBuffer;
	const Vertex vertices[3] = {
		{{ -0.5f, -0.5f, 0.0f}, {1.0,0,0}},
		{ {0.0f,  0.5f, 0.0f}, {0,1.0,0} },
		{{0.5f, -0.5f, 0.0f}, {0.0,0,1.0}},
	};
};
class CRenderer {
public:
	static CRenderer& Get() {
		static CRenderer s_Renderer;
		return s_Renderer;
	}
	void InitDeviceLayer() {
		lagVertexShader::sm_Device = this->m_pDevice;
		lagFragmentShader::sm_Device = this->m_pDevice;
		lagVertexBuffer::sm_Device = this->m_pDevice;

	}
	void Init(const CWindow* wind) {
		auto devs = sOutDevice::Init(wind->GetWindowHandle());
		this->m_pSwapChain = devs->m_pSwapChain;
		this->m_pDevice = devs->m_pDevice;
		this->m_pContext = devs->m_pContext;
		delete devs; // devs does not exist anymore. 
		InitDeviceLayer();
		HRESULT hr{};
		ID3D11Buffer* m_Backbuffer = nullptr;
		hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_Backbuffer));
		if (FAILED(hr)) {
			std::cout << "could not get bb" << hr;
			return;
		}
		hr = m_pDevice->CreateRenderTargetView(m_Backbuffer, nullptr, &m_View);
		if (FAILED(hr)) {
			std::cout << "could not create rtv" << hr;
			m_Backbuffer->Release();
			return;
		}
		m_Backbuffer->Release();
		m_pContext->OMSetRenderTargets(1, &m_View, NULL); // NO DEPTH.
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		RECT rc{};
		GetClientRect(wind->GetWindowHandle(), &rc);
		viewport.Width = rc.right - rc.left;
		viewport.Height = rc.bottom - rc.top;
		m_pContext->RSSetViewports(1, &viewport);
		m_VertexBuffer = new lagVertexBuffer();


		D3D11_RASTERIZER_DESC rastDesc{};
		rastDesc.FillMode = D3D11_FILL_SOLID;
		rastDesc.CullMode = D3D11_CULL_BACK; // Disable culling to avoid orientation issues
		rastDesc.DepthClipEnable = TRUE;

		ID3D11RasterizerState* rastState = nullptr;
		hr = m_pDevice->CreateRasterizerState(&rastDesc, &rastState);

		m_pContext->RSSetState(rastState);


		if (FAILED(hr)) {
			std::cout << "[BUFFER]" << hr << std::endl;
			return;
		}
		CompileShaders();
	} 

	void CompileShaders() {
		HRESULT hr;
		m_VertexShader = new lagVertexShader(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\vs.hlsl", "main", D3DCOMPILE_DEBUG);
		m_FragShader = new lagFragmentShader(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\ps.hlsl", "Main", D3DCOMPILE_DEBUG);
		D3D11_INPUT_ELEMENT_DESC element1{};
		element1.SemanticName = "POSITION";
		element1.SemanticIndex = 0;
		element1.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		element1.InputSlot = 0;
		element1.AlignedByteOffset = 0;
		element1.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		element1.InstanceDataStepRate = 0;
		D3D11_INPUT_ELEMENT_DESC element2{};
		element2.SemanticName = "COLOR";
		element2.SemanticIndex = 0;
		element2.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		element2.InputSlot = 0;
		element2.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; // im sure this won't cause issues. (foreshadowing)
		element2.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		element2.InstanceDataStepRate = 0;

		D3D11_INPUT_ELEMENT_DESC Desc[2] = { element1, element2 }; 
		hr = m_pDevice->CreateInputLayout(Desc, 2, m_VertexShader->GetBlob()->GetBufferPointer(), m_VertexShader->GetBlob()->GetBufferSize(), &m_Layout); // 
		if (FAILED(hr)) {
			std::cout << hr << std::endl;
			return;
		}
	}
	
	void Update() {
		const float m_fClear[4] = { 0.145,0.145,0.16,1.0f };
		m_pContext->ClearRenderTargetView(m_View, m_fClear);
		m_VertexBuffer->Bind(m_pContext); 

		m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // ?
		m_pContext->IASetInputLayout(m_Layout);

		m_pContext->PSSetShader(this->m_FragShader->GetShader(), nullptr, 0); // Signature Similar?
		m_pContext->VSSetShader(this->m_VertexShader->GetShader(), nullptr, 0);
		//test.
		m_pContext->Draw(3, 0);
		m_pSwapChain->Present(1, 0);
	}
	void Shutdown() {
		delete m_VertexBuffer;
		delete m_VertexShader;
		delete m_FragShader;
		if(m_View) m_View->Release();
		if(m_pDevice) m_pDevice->Release();
		if(m_pSwapChain) m_pSwapChain->Release();
		if(m_pContext) m_pContext->Release();
	}
private:
	const Vertex vertices[3] = {
		{{ -0.5f, -0.5f, 0.0f}, {1.0,0,0}},
		{ {0.0f,  0.5f, 0.0f}, {0,1.0,0} },
		{{0.5f, -0.5f, 0.0f}, {0.0,0,1.0}},
	};
	ID3D11InputLayout* m_Layout = nullptr;
	
	lagVertexShader* m_VertexShader = nullptr;
	lagFragmentShader* m_FragShader = nullptr;
	lagVertexBuffer* m_VertexBuffer = nullptr;

	D3D11_VIEWPORT viewport;
	ID3D11RenderTargetView* m_View = nullptr;
	IDXGISwapChain* m_pSwapChain = nullptr;
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pContext = nullptr;
};
class CApplication {
public:
	static CApplication& Get() {
		static CApplication m; return m;
	}
	void Init() {
#ifdef LE_WIN
		m_Arguments = (CWinArgs*)legit::g_Args;
		CLogger::Init();
		CWindowEvents::InitClass(); // Has to happen before the Window class Inits. 
		m_Window = new CMainWindow(m_Arguments);
		CRenderer::Get().Init(m_Window->GetRawWindow());
#else
		printf("[Error]: Unknown/Unanticipated Platform. We are unable to support your operating system or platform.");
		this->m_bStopRunning = true;
#endif // LE_WIN
		return;
	}
	void Update() {
		this->m_bStopRunning = m_Window->g_Close;
		m_Window->Poll();
		CRenderer::Get().Update();
	}
	void Run() {
		Init();
		while (!m_bStopRunning) {
			Update();
		}
		Shutdown();
	}
	void Shutdown() {
		CRenderer::Get().Shutdown();
		delete m_Window;
		CLogger::Shutdown();
	}
	CMainWindow* GetWindow() { return m_Window; }
private:
	CMainWindow* m_Window = nullptr;
#ifdef LE_WIN32
	CWinArgs*
#else 
	legit::fwCmdArgs*
#endif
	m_Arguments = nullptr;
	bool m_bStopRunning = false;
};