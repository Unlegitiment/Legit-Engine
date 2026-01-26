#include "stb_image.h"

#include <iostream>
#include <LECore/core/args.h>
#include <LECore/headers/platform_specs.h>
#include <string>
#include "WindowHandling/WindowsCmdArgs.h"
using namespace legit;

#ifdef LE_WIN
#include <Windows.h>

#include "lagGraphics.h"
#include <DirectXMath.h>

#include <chrono>
class CTimer {
public:
	using Clock = std::chrono::steady_clock;
	static void Start() {
		StartTime = Clock::now();
		LastFrame = StartTime;
	}
	static void Tick() {
		auto now = Clock::now();
		Delta = std::chrono::duration<double>(now - LastFrame).count();
		Total = std::chrono::duration<double>(now - StartTime).count();
		LastFrame = now;
	}
	static double GetDeltaSeconds() {
		return Delta;
	}
	static double GetTotalSeconds() {
		return Total;
	}
private:
	static inline Clock::time_point StartTime{};
	static inline Clock::time_point LastFrame{};
	static inline double Delta = 0.0f;
	static inline double Total = 0.0f;
};


using Instance = HINSTANCE;
using WindowsProcess = LRESULT(*)(HWND, UINT, WPARAM, LPARAM);
struct WindowClassDesc {
	WindowsProcess Process{};
	const wchar_t* WindowClassName;
	Instance hInstance;
};
class CWindowClass {
public:
	static WNDCLASS ConvertToWindowClass(const WindowClassDesc& Desc) {
		WNDCLASS c{};
		c.lpfnWndProc = Desc.Process;
		c.lpszClassName = Desc.WindowClassName;
		c.hInstance = Desc.hInstance;
		return c;
	}
public:
	CWindowClass(WindowClassDesc Desc) : m_Desc(Desc){
		auto a = ConvertToWindowClass(Desc);
		RegisterClass(&a);
	}
	const WindowClassDesc& GetDescription() {
		return this->m_Desc;
	}
	~CWindowClass() {
		UnregisterClass(m_Desc.WindowClassName, m_Desc.hInstance);
	}
private:
	WindowClassDesc m_Desc{};
};
struct WindowDescription {
	const wchar_t* WindowName;
};
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
class CWindow {
public:
	explicit CWindow(CWindowClass* WindClass, WindowDescription Desc) : m_Class(WindClass){
		m_pWindowHandle = CreateWindowEx(0, WindClass->GetDescription().WindowClassName, Desc.WindowName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, WindClass->GetDescription().hInstance, NULL);
		if (m_pWindowHandle == NULL) {
			int Error = GetLastError();
			__debugbreak(); // we fucked up.
		}
	}
	void Show(int nCmdShow = SW_SHOW) const {
		ShowWindow(this->m_pWindowHandle, nCmdShow);
	}
	bool Peek(MSG& uMsg, DWORD QueueUpdate = PM_REMOVE) {
		return PeekMessage(&uMsg, NULL, 0, 0, QueueUpdate);
	}
	int GetMaxSysMsgs() {
		return WM_APP - 1;
	}
	HWND GetHandle() {
		return this->m_pWindowHandle;
	}
	~CWindow() {
		
	}
private:
	CWindowClass* m_Class{};
	HWND m_pWindowHandle = nullptr;
};

#define DEFINE_HANDLER(x) LRESULT Handle##x(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

DEFINE_HANDLER(Close) {
	OutputDebugStringA("[WINDOWSSPROCESS] --WM_CLOSE Triggered--\n");
	int ApplicationIdentifier = MessageBox(NULL, L"Would you like to Quit?", L"Quit?", MB_YESNO);
	char Buffer[1028] = {0};
	sprintf_s(Buffer, "ApplicationId: %d\n", ApplicationIdentifier);
	OutputDebugStringA(Buffer);
	if (ApplicationIdentifier == IDYES) {
		PostQuitMessage(0);
	}
	return 0;
}

DEFINE_HANDLER(Input) {
	OutputDebugStringA(__FUNCTION__ "\n");
	return 0;
}
#undef DEFINE_HANDLER
#define ProcDef(WndMsg, Func) case WndMsg: return Func(wnd, msg, wP, lParam);
#define ProcDefault() default: DefWindowProc(wnd, msg, wP, lParam);
static LRESULT LE_Process(HWND wnd, UINT msg, WPARAM wP, LPARAM lParam) { // The developers who made this garbage should be killed. God I hate it. 
	if (ImGui_ImplWin32_WndProcHandler(wnd, msg, wP, lParam)) {
		return true;
	}
	switch (msg) {
		ProcDef(WM_CLOSE, HandleClose);
		ProcDef(WM_INPUT, HandleInput);
		ProcDefault();
	}
}
#undef ProcDef
#undef ProcDefault

class CGameWindow {
public:
	static void Init() {
		WindowClassDesc desc{};
		desc.hInstance = GetModuleHandle(0);
		desc.Process = LE_Process;
		desc.WindowClassName = L"WindowClass";
		m_Class = new CWindowClass(desc);
		WindowDescription wind{};
		wind.WindowName = L"Test";
		m_Window = new CWindow(m_Class, wind);
		m_Window->Show();
	}
	static bool Update() {
		MSG m{};
		if (m_Window->Peek(m)) {
			TranslateMessage(&m);
			DispatchMessage(&m);
			if (m.message == WM_QUIT) {
				return true;
			}
		}
		return false;
	}
	static void Destroy() {
		delete m_Window;
		delete m_Class;
	}
	static CWindow* GetWindow() {
		return m_Window;
	}
	static CWindowClass* GetWindowClass() {
		return m_Class;
	}
	CGameWindow& operator=(const CGameWindow&) noexcept = delete;
private:
	static inline CWindowClass* m_Class{nullptr};
	static inline CWindow* m_Window{nullptr};
};
template<typename T, size_t Size> constexpr size_t ArrayBytes(const T(&Op)[Size]) {
	return sizeof(T) * Size;
}
class CBaseEntity {
public:
	void SetPosition(DirectX::XMFLOAT3 Position) {
		m_Position = Position;
	}
	void SetRotation(DirectX::XMFLOAT3 Rotation) {
		m_Rotation = Rotation;
	}
	void SetScale(DirectX::XMFLOAT3 Scale) {
		m_Scale = Scale;
	}
	DirectX::XMFLOAT3 GetScale() {
		return m_Scale;
	}
	DirectX::XMFLOAT3 GetPosition() {
		return m_Position;
	}
	DirectX::XMFLOAT3 GetRotation() {
		return m_Rotation;
	}
protected:
	DirectX::XMFLOAT3 m_Position, m_Rotation, m_Scale;
};
class CPhysicalQuad : public CBaseEntity{
public:
	CPhysicalQuad() {
		m_Position = DirectX::XMFLOAT3(0, 0, 0); m_Rotation = DirectX::XMFLOAT3(0, 0, 0); m_Scale = DirectX::XMFLOAT3(1, 1, 1);
		D3D11_BUFFER_DESC m_Desc{};
		m_Desc.Usage = D3D11_USAGE_IMMUTABLE;
		m_Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		m_Desc.ByteWidth = ArrayBytes(quadVertices);
		D3D11_SUBRESOURCE_DATA Data{};
		Data.pSysMem = quadVertices;
		Buffer = lagGraphics::CreateBuffer(&m_Desc, &Data);
		lagShaderCompiler Comp{};
		
		lagByteCode Byte = Comp.Compile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\lag_basic.hlsl", "VS_Main", "vs_5_0").GetReturn();
		VSShader = lagGraphics::CreateVertexShader(Byte.Byte);
		D3D11_INPUT_ELEMENT_DESC Desc{};
		Desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		Desc.SemanticName = "POSITION";
		Desc.SemanticIndex = 0;
		Desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		Desc.InputSlot = 0;
		Desc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		D3D11_INPUT_ELEMENT_DESC Desc2{};
		Desc2.Format = DXGI_FORMAT_R32G32_FLOAT;
		Desc2.SemanticName = "UVCOORD";
		Desc2.SemanticIndex = 0;
		Desc2.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		Desc2.InputSlot = 0;
		Desc2.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		InputAssembler = lagGraphics::CreateInputAssembler({Desc, Desc2}, Byte.Byte);

		Byte = Comp.Compile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\lag_basic.hlsl", "PS_Main", "ps_5_0").GetReturn();
		PSShader = lagGraphics::CreateFragmentShader(Byte.Byte);
	}
	DirectX::XMMATRIX GetMatrix() const {
		auto x = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&m_Position))* DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&m_Rotation))* DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&m_Scale));
		x = DirectX::XMMatrixTranspose(x);
		return x;
	}
	void Draw() {
		const UINT stride = sizeof(float) * 5;
		const UINT offset = 0;

		//lagGraphics::SetFragmentShaderResources(0, {SRV});
		lagGraphics::SetVertexBuffers(0, {Buffer}, &stride, &offset);
		lagGraphics::SetFragmentShader(PSShader);
		lagGraphics::SetVertexShader(VSShader);
		lagGraphics::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		lagGraphics::SetInputAssembler(InputAssembler);
		lagGraphics::Draw(6, 0);
	}
	~CPhysicalQuad() {

	}
private:
	lagInputAssembler* InputAssembler;
	lagVertexShader* VSShader;
	lagFragmentShader* PSShader;
	lagBuffer* Buffer;
	const float quadVertices[30] = {
		// positions       // flipped texCoords
		-1.0f, -1.0f,0,      0.0f, 1.0f,
		-1.0f,  1.0f,0,      0.0f, 0.0f,
		 1.0f, -1.0f,0,      1.0f, 1.0f,
		-1.0f,  1.0f,0,      0.0f, 0.0f,
		 1.0f,  1.0f,0,      1.0f, 0.0f,
		 1.0f, -1.0f,0,      1.0f, 1.0f
	};
};
class CBillboard : public CPhysicalQuad{
public:
	CBillboard() {
		const char* Path = "E:\\WIN_BACKUP\\PICS\\Screenshots\\Screenshot 2024-02-25 164512.png";

		int x, y, w;
		stbi_uc* buff = stbi_load(Path, &x, &y, &w, 4);
		float fImageAspect = (float)x / (float)y;
		float fScalar = 3.0f;
		CPhysicalQuad::SetScale(DirectX::XMFLOAT3(fImageAspect * fScalar, 1.0 * fScalar, 1));
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = x;
		textureDesc.Height = y;
		textureDesc.MipLevels = 1; // im not gonna abstract these yet.
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		D3D11_SUBRESOURCE_DATA textureResourceData{};
		textureResourceData.pSysMem = buff;
		textureResourceData.SysMemPitch = x * sizeof(UINT);
		m_pTexture = lagGraphics::CreateTexture2D(&textureDesc, &textureResourceData);
		this->SRV = lagGraphics::CreateShaderResource(m_pTexture, nullptr);
		stbi_image_free(buff);
		this->fAspect = fImageAspect;
	}
	void Draw() {
		lagGraphics::SetFragmentShaderResources(0, {SRV});
		lagGraphics::SetFragmentShaderSamplers(0, {nullptr});
		CPhysicalQuad::Draw();
	}
	void SetScale(DirectX::XMFLOAT3 scalar) {
		CPhysicalQuad::SetScale({scalar.x * fAspect, scalar.y * fAspect, 0.0f});
	}
private:
	float fAspect = 0;
	lagTexture2D* m_pTexture = nullptr;
	lagShaderResource* SRV = nullptr;
};
class CCube : public CBaseEntity{
public:
	CCube() {
		m_Position = DirectX::XMFLOAT3(0, 0, 0); m_Rotation = DirectX::XMFLOAT3(0, 0, 0); m_Scale = DirectX::XMFLOAT3(1, 1, 1);
		D3D11_BUFFER_DESC m_Desc{};
		m_Desc.Usage = D3D11_USAGE_IMMUTABLE;
		m_Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		m_Desc.ByteWidth = ArrayBytes(Vertices);
		D3D11_SUBRESOURCE_DATA Data{};
		Data.pSysMem = Vertices;
		Buffer = lagGraphics::CreateBuffer(&m_Desc, &Data);
		lagShaderCompiler Comp{};
		lagByteCode Byte = Comp.Compile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\lag_basic.hlsl", "VS_Main", "vs_5_0").GetReturn();
		VSShader = lagGraphics::CreateVertexShader(Byte.Byte);

		D3D11_INPUT_ELEMENT_DESC Desc{};
		Desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		Desc.SemanticName = "POSITION";
		Desc.SemanticIndex = 0;
		Desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		Desc.InputSlot = 0;
		Desc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		D3D11_INPUT_ELEMENT_DESC Desc2{};
		Desc2.Format = DXGI_FORMAT_R32G32_FLOAT;
		Desc2.SemanticName = "UVCOORD";
		Desc2.SemanticIndex = 0;
		Desc2.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		Desc2.InputSlot = 0;
		Desc2.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		InputAssembler = lagGraphics::CreateInputAssembler({Desc, Desc2}, Byte.Byte);

		Byte = Comp.Compile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\lag_basic.hlsl", "PS_Main", "ps_5_0").GetReturn();
		PSShader = lagGraphics::CreateFragmentShader(Byte.Byte);
	}
	DirectX::XMMATRIX GetMatrix() const {
		return DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&m_Position)) * DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&m_Rotation)) * DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&m_Scale));
	}
	void Draw() {
		const UINT stride = sizeof(float) * 5;
		const UINT offset = 0;

		lagGraphics::SetVertexBuffers(0, {Buffer}, &stride, &offset);
		lagGraphics::SetFragmentShader(PSShader);
		lagGraphics::SetVertexShader(VSShader);
		lagGraphics::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		lagGraphics::SetInputAssembler(InputAssembler);
		lagGraphics::Draw(36, 0);
	}
private:
	lagInputAssembler* InputAssembler = nullptr;
	lagVertexShader* VSShader = nullptr;
	lagFragmentShader* PSShader = nullptr;
	lagBuffer* Buffer = nullptr;
	float Vertices[36 * 5] = {
	// Back face (z = -0.5)
	-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,   1.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,   1.0f, 1.0f,

	-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,   0.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,   1.0f, 0.0f,

	// Front face (z = +0.5)
	-0.5f, -0.5f,  0.5f,   0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,   1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,   1.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,   0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,   0.0f, 0.0f,

	// Left face (x = -0.5)
	-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,   1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,   1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,   0.0f, 0.0f,

	// Right face (x = +0.5)
	 0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,   1.0f, 1.0f,

	 0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,   0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,   1.0f, 0.0f,

	// Top face (y = +0.5)
	-0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,   0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,   1.0f, 0.0f,

	-0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,   1.0f, 1.0f,

	// Bottom face (y = -0.5)
	-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,   1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,   1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,   1.0f, 0.0f,
	};
};
class CTexturedCube : public CCube{
public:
	CTexturedCube() {
		const char* Path = "E:\\WIN_BACKUP\\PICS\\Screenshots\\Screenshot 2024-02-25 164512.png";

		int x, y, w;
		stbi_uc* buff = stbi_load(Path, &x, &y, &w, 4);
		float fImageAspect = (float)x / (float)y;
		float fScalar = 2.5f;
		SetScale(DirectX::XMFLOAT3(fImageAspect * fScalar, 1.0 * fScalar, fImageAspect * fScalar));
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = x;
		textureDesc.Height = y;
		textureDesc.MipLevels = 1; // im not gonna abstract these yet.
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		D3D11_SUBRESOURCE_DATA textureResourceData{};
		textureResourceData.pSysMem = buff;
		textureResourceData.SysMemPitch = x * sizeof(UINT);
		m_pTexture = lagGraphics::CreateTexture2D(&textureDesc, &textureResourceData);
		this->SRV = lagGraphics::CreateShaderResource(m_pTexture, nullptr);
		stbi_image_free(buff);
		this->fAspect = fImageAspect;
	}
	void Draw() {
		lagGraphics::SetFragmentShaderResources(0, {SRV});
		CCube::Draw();
	}
private:
	lagTexture2D* m_pTexture;
	lagShaderResource* SRV;
	float fAspect{};
};

struct LAGMatrices {
	DirectX::XMMATRIX m_Projection;
	DirectX::XMMATRIX m_View;
	DirectX::XMMATRIX m_Model;
};
class CDefaultPass {
public:
	static void Init(HWND Window) {
		sm_pRTV = lagGraphics::CreateRenderTarget(lagGraphics::GetBackBuffer());
		D3D11_TEXTURE2D_DESC depthBufferDesc{};
		lagTexture2D* BB = (lagTexture2D*)lagGraphics::GetBackBuffer();
		BB->GetDesc(&depthBufferDesc);
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		lagTexture2D* DepthTexture = lagGraphics::CreateTexture2D(&depthBufferDesc, nullptr);
		sm_pDTV = lagGraphics::CreateDepthView(DepthTexture, nullptr);
		D3D11_DEPTH_STENCIL_DESC DSDesc{};
		DSDesc.DepthEnable = true;
		DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		DSDesc.DepthFunc = D3D11_COMPARISON_GREATER;
		sm_pDSS = lagGraphics::CreateDepthState(&DSDesc);
		lagGraphics::SetDepthState(sm_pDSS, 0);
		float quadVertices[] = {
			// positions       // flipped texCoords
			-1.0f, -1.0f,0.0,      0.0f, 1.0f,
			-1.0f,  1.0f,0.0,      0.0f, 0.0f,
			 1.0f, -1.0f,0.0,      1.0f, 1.0f,
			-1.0f,  1.0f,0.0,      0.0f, 0.0f,
			 1.0f,  1.0f,0.0,      1.0f, 0.0f,
			 1.0f, -1.0f,0.0,      1.0f, 1.0f
		};
		D3D11_RASTERIZER_DESC RasterDesc{};
		RasterDesc.CullMode = D3D11_CULL_NONE;
		RasterDesc.DepthClipEnable = true;
		RasterDesc.FillMode = D3D11_FILL_SOLID;
		lagRasterizerState* pState{};
		pState = lagGraphics::CreateRasterizationState(&RasterDesc);
		lagGraphics::SetRasterizerState(pState);

		D3D11_VIEWPORT Port{};
		Port.TopLeftX = 0;
		Port.TopLeftY = 0;
		RECT rc{};
		GetClientRect(Window, &rc);
		Port.Width = rc.right - rc.left;
		Port.Height = rc.bottom - rc.top;
		lagGraphics::SetViewports({Port});


		lagGraphics::SetRenderTargets({sm_pRTV}, sm_pDTV);

		D3D11_BUFFER_DESC BufferDesc{};
		BufferDesc.ByteWidth = sizeof(LAGMatrices);
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		LAGConstants = lagGraphics::CreateBuffer(&BufferDesc, nullptr);
		Quad = new CBillboard();
		Cube = new CTexturedCube();
	}
	static inline lagBuffer* LAGConstants = nullptr;
	static inline CBillboard* Quad = nullptr;
	static inline CTexturedCube* Cube = nullptr;
	static void SetupCamera(LAGMatrices* Mats) {
		Mats->m_View = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH({4,2.5,-5}, {0,0,0}, {0,1,0}));
		Mats->m_Projection = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f), 16.0f / 9.0f, 0.01f, 250));
	}
	template<typename T> static void RenderItem(LAGMatrices* Constants, T* item) {
		Constants->m_Model = item->GetMatrix();
		LAGMatrices* mats{nullptr};
		mats = (LAGMatrices*)lagGraphics::Map(LAGConstants, 0, D3D11_MAP_WRITE_DISCARD, 0);
		memcpy(mats, Constants, sizeof(LAGMatrices));
		lagGraphics::Unmap(LAGConstants, 0);
		lagGraphics::SetVertexShaderBuffers(0, {LAGConstants});


		item->Draw();
	}
	static void Process() {
		float f[4] = {0.5,0,0,1};
		lagGraphics::ClearRenderTarget(sm_pRTV, f);
		lagGraphics::ClearDepthView(sm_pDTV, D3D11_CLEAR_DEPTH, 1.0, 0);
		LAGMatrices Constants{};
		LAGMatrices* mats = nullptr;
		SetupCamera(&Constants);

		//Quad->SetPosition({0,0,2});
		
		//Cube->SetPosition({0,0,0});


		RenderItem(&Constants, Quad);

		RenderItem(&Constants, Cube);

		lagGraphics::Present(1, 0);
	}
	static void Destroy() {
		lagGraphics::Shutdown();
	}
private:
	static inline lagRenderTarget* sm_pRTV{nullptr};
	static inline lagDepthView* sm_pDTV{nullptr};
	static inline lagDepthState* sm_pDSS{nullptr};
};
class CQuadVertices {
public:
	const float quadVertices[30] = {
		// positions       // flipped texCoords
		-1.0f, -1.0f,0,      0.0f, 1.0f,
		-1.0f,  1.0f,0,      0.0f, 0.0f,
		 1.0f, -1.0f,0,      1.0f, 1.0f,
		-1.0f,  1.0f,0,      0.0f, 0.0f,
		 1.0f,  1.0f,0,      1.0f, 0.0f,
		 1.0f, -1.0f,0,      1.0f, 1.0f
	};
	CQuadVertices(){
		D3D11_BUFFER_DESC m_Dsc{};
		m_Dsc.ByteWidth = sizeof(quadVertices);
		m_Dsc.Usage = D3D11_USAGE_IMMUTABLE;
		m_Dsc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA data{};
		data.pSysMem = quadVertices;
		this->Vertices = lagGraphics::CreateBuffer(&m_Dsc, &data);
	}
	void Bind(const unsigned int* Stride, const unsigned int* Offset) {
		lagGraphics::SetVertexBuffers(0, {Vertices}, Stride, Offset);
	}
	~CQuadVertices() {

	}
private:
	lagBuffer* Vertices{};
};
class ShaderComp : private lagShaderCompiler{
private:
	lagByteCode _Compile(const wchar_t* Path, const char* Entry, const char* Ver = "vs_5_0") {
		auto CompRes = lagShaderCompiler::Compile(Path, Entry, Ver);
		
		if (CompRes.HasFailed() && CompRes.GetOperationReturn().IsReturnError()) {
			
		}
		else {
			return CompRes.GetReturn();
		}
	}
public:
	static lagByteCode Compile(const wchar_t* Path, const char* Entry, const char* Ver = "vs_5_0") {
		ShaderComp Comp{};
		return Comp._Compile(Path, Entry, Ver);
	}
};
enum class eBindStage {
	CONSTANT_BUFFER = 0x4L,
	VERTEX_BUFFER = 0x1L,
	INDEX_BUFFER = 0x2L,
	SHADER_RESOURCE = 0x8L,
	STREAM_OUTPUT = 0x10L,
	RENDER_TARGET = 0x20L,
	DEPTH_STENCIL = 0x40L,
	UNORDERED_ACCESS = 0x80L,
	DECODER = 0x200L,
	VIDEO_ENCODER = 0x400L,
	MAX = 0x400L + 1
};
/*Right now the conversion technically works just because it maps perfectly to the D3D11 Type. But seriously don't consider it lmao.*/
enum class eUsage {
	DEFAULT,
	IMMUTABLE,
	DYNAMIC,
	STAGING,
	MAX_USAGE
};
class gpuConversion {
public:
	struct BindStage {
		struct MappingType {
			static D3D11_BIND_FLAG Get(eBindStage Stage) {

			}
		};
	};
};
class gpuBuffer {
public:
	gpuBuffer(){
		m_Buffer = nullptr;
	}
	gpuBuffer(size_t SizeOf, eBindStage BindStage, eUsage Usage, unsigned int CPUAccess) {
		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = SizeOf;
		desc.Usage = (D3D11_USAGE)Usage; // it maps
		desc.BindFlags = (D3D11_BIND_FLAG)BindStage; // maps
		desc.CPUAccessFlags = CPUAccess;
		m_Buffer = lagGraphics::CreateBuffer(&desc, nullptr);
		if (m_Buffer == nullptr) {
			std::cout << "gpuBuffer, Failed to Initialize Buffer Instance!";
			__debugbreak();
		}
	}
	lagBuffer* GetBuffer() {
		return this->m_Buffer;
	}
	~gpuBuffer() {
		if (m_Buffer) m_Buffer->Release();
	}
private:
	lagBuffer* m_Buffer;
};
/*
	If theres one thing that I've learned is that you can make any abstraction "decent" just as long as the original is so primitive that its possible.
	Also that you get more of an idea of how things should be worked into things whether they should be global or not. (I primarily am talking about CGameWindow)
	CGameWindow is a perfect example, as its a item that is general to the application and wouldn't be reusable as a result, but also it uses an abstraction and as well is a global.
	If I eventually find that I don't actually need it, I can remove it and add it to just a renderer or something smaller.
	But like as an example ShaderComp as well is a match too, lagShaderCompiler wasn't originally a "global" system however ShaderComp proves that its much more viable as one, thus on the next iteration of the design, we just make it static.
*/
class lagUpdatableResource {
public:
	virtual void Update() = 0;
private:
};
/* 
	The V-Table fucks with the values of the internal stuff, and thus also messes with the offset. 
	So just define it like a standard template type and it functions like a proxy anyways lmao
*/

template<typename T>
class UpdatableBuffer : public lagUpdatableResource{
public:
	UpdatableBuffer() : m_Size(sizeof(T)){
		Buffer = new char[sizeof(T)] {0};
		D3D11_BUFFER_DESC Desc{};
		Desc.ByteWidth = sizeof(T);
		Desc.Usage = D3D11_USAGE_DYNAMIC;
		Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pBuff = lagGraphics::CreateBuffer(&Desc, nullptr);
	}
	UpdatableBuffer(const UpdatableBuffer<T>&) = delete;
	UpdatableBuffer& operator=(const UpdatableBuffer<T>&) = delete; // you shouldn't be able to "copy" a gpu resource. (or if you can, why would you lmao?)
	UpdatableBuffer& operator=(UpdatableBuffer<T>&&) = delete; // moving a resource isn't really a thing. but you could theoretically write one if it needed to leave the one instance, but other than that not really that useful lmao. 
	UpdatableBuffer(UpdatableBuffer<T>&&) = delete;
	void Update() {
		void* v = lagGraphics::Map(pBuff, 0, D3D11_MAP_WRITE_DISCARD, 0);
		memcpy(v, Buffer, m_Size);
		lagGraphics::Unmap(pBuff, 0);
	}
	T* operator->() {
		return (T*)Buffer; // i wonder if that would work  *fucking explodes*
	}
	T* As() {
		assert(sizeof(T) == m_Size && "Sizes do not equal");
		return (T*)Buffer;
	}
	lagBuffer* GetGPU() {
		return this->pBuff;
	}
	~UpdatableBuffer() {
		delete Buffer;
		if(pBuff) pBuff->Release();
	}
private:
	char* Buffer = nullptr;
	size_t m_Size;
	lagBuffer* pBuff{};
};
class CDoFancyShaderStuff {
public:
	struct alignas(16) CBuff{
		DirectX::XMFLOAT2 Res;
		float iTime;
	};
	CDoFancyShaderStuff() {
		Target = lagGraphics::CreateRenderTarget(lagGraphics::GetBackBuffer());
		lagGraphics::SetRenderTargets({Target}, nullptr);
		lagByteCode vsPass = ShaderComp::Compile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\vs_full.hlsl", "VS");
		D3D11_INPUT_ELEMENT_DESC Desc{};
		Desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		Desc.SemanticName = "POSITION";
		Desc.SemanticIndex = 0;
		Desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		Desc.InputSlot = 0;
		Desc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		D3D11_INPUT_ELEMENT_DESC Desc2{};
		Desc2.Format = DXGI_FORMAT_R32G32_FLOAT;
		Desc2.SemanticName = "UVCOORD";
		Desc2.SemanticIndex = 0;
		Desc2.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		Desc2.InputSlot = 0;
		Desc2.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		ia = lagGraphics::CreateInputAssembler({Desc, Desc2}, vsPass.Byte);
		lagVertexShader* Shader = lagGraphics::CreateVertexShader(vsPass.Byte);
		lagGraphics::SetInputAssembler(ia);
		lagGraphics::SetVertexShader(Shader);
		unsigned int Stride = 5 * sizeof(float); // offset based on Input Assembler
		unsigned int Offset = 0;
		Verts.Bind(&Stride, &Offset);
		auto a = ShaderComp::Compile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\ps_full.hlsl", "PS", "ps_5_0");
		lagFragmentShader* pShader = lagGraphics::CreateFragmentShader(a.Byte);
		lagGraphics::SetFragmentShader(pShader);
		lagGraphics::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		lagTexture2D* pTexture = (lagTexture2D*)lagGraphics::GetBackBuffer();
		D3D11_TEXTURE2D_DESC textureDesc{};
		pTexture->GetDesc(&textureDesc);
		fTextureWidth = textureDesc.Width; fTextureHeight = (float)textureDesc.Height;
	}
	void Process() {
		float fClear[4]{0,0,0,1};
		lagGraphics::ClearRenderTarget(Target, fClear);
		CBuffer->Res = {fTextureWidth, fTextureHeight};
		CBuffer->iTime = CTimer::GetTotalSeconds();
		CBuffer.Update();
		lagGraphics::SetFragmentShaderBuffers(0, {CBuffer.GetGPU()});
		lagGraphics::Draw(6, 0);
	}
	~CDoFancyShaderStuff() {

	}
private:
	UpdatableBuffer<CBuff> CBuffer{};
	float fTextureWidth, fTextureHeight;
	CQuadVertices Verts{};
	lagInputAssembler* ia{nullptr};
	lagRenderTarget* Target{nullptr};
};

class CImGui {
public:
	static void Init() {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_DockingEnable;
		ImGui_ImplWin32_Init(CGameWindow::GetWindow()->GetHandle());
		ImGui_ImplDX11_Init(lagGraphics::GetDeviceHandle(), lagGraphics::GetDeviceContext());
	}
	static void BeginFrame() {
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		m_IsGuiRunning = true;
	}
	static bool IsGuiActive() {
		return m_IsGuiRunning;
	}
	static void EndFrame() {
		m_IsGuiRunning = false;
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
	static void Destroy() {
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
private:
	static inline bool m_IsGuiRunning = false;
};
#include "DiscordApplication.h"
class CApplication {
public:
	static void Init() {
		CTimer::Start();
		CGameWindow::Init();
		lagGraphics::Init(CGameWindow::GetWindow()->GetHandle());
		lagGraphics::SetDebuggerActive();
		CImGui::Init();
		ShaderPass = new CDoFancyShaderStuff();
		lagGraphics::SetViewports({lagGraphics::GetClientViewport(CGameWindow::GetWindow()->GetHandle())}); // i hate viewports but they are kinda cool
		richDiscord::Init();
	}
	static bool Update() {
		CImGui::BeginFrame();
		CTimer::Tick();
		richDiscord::Update();
		bool DoesWindowWantToClose = CGameWindow::Update();
		ShaderPass->Process();
		CImGui::EndFrame();
		lagGraphics::Present(1, 0);
		return DoesWindowWantToClose;
	}
	static void Shutdown() {
		richDiscord::Shutdown();
		delete ShaderPass; ShaderPass = nullptr;
		CImGui::Destroy();
		lagGraphics::Shutdown();
		CGameWindow::Destroy();
	}
private:
	static inline CDoFancyShaderStuff* ShaderPass = nullptr;
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	CWinArgs args = CWinArgs{hInstance, hPrevInstance, pCmdLine, nCmdShow};



	CApplication::Init();
	while (!CApplication::Update()) {

	}
	CApplication::Shutdown();
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