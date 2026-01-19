#include "stb_image.h"

#include <iostream>
#include <LECore/core/args.h>
#include <LECore/headers/platform_specs.h>
#include <string>
#include "WindowHandling/WindowsCmdArgs.h"
using namespace legit;

#ifdef LE_WIN
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
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
#include <vector>
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
	switch (msg) {
		ProcDef(WM_CLOSE, HandleClose);
		ProcDef(WM_INPUT, HandleInput);
		ProcDefault();
	}
}
#undef ProcDef
#undef ProcDefault
struct lagByteCode {
	std::vector<char> Byte;
};
class lagShaderCompiler {
public:
	lagByteCode Compile(const wchar_t* ShaderPath, const char* ShaderEntry, const char* ShaderVersion) {
		ID3DBlob* pBlob, *pErr{};
		HRESULT hr = D3DCompileFromFile(ShaderPath, NULL, NULL, ShaderEntry, ShaderVersion, 0, 0, &pBlob, &pErr);
		if (FAILED(hr)) {
			OutputDebugStringA((char*)pErr->GetBufferPointer());
			OutputDebugStringA("\n");
			__debugbreak();
		}
		lagByteCode c{};
		c.Byte.resize(pBlob->GetBufferSize());
		memcpy(c.Byte.data(), pBlob->GetBufferPointer(), sizeof(char) * pBlob->GetBufferSize());
		return c;
	}
private:

};
using lagDeviceHandle = ID3D11Device;
using lagDeviceContext = ID3D11DeviceContext;
using lagSwapChain = IDXGISwapChain;
using lagResource = ID3D11Resource;

using lagVertexShader = ID3D11VertexShader;
using lagFragmentShader = ID3D11PixelShader;
using lagDomainShader = ID3D11DomainShader;
using lagGeometryShader = ID3D11GeometryShader;
using lagHullShader = ID3D11HullShader;

using lagRenderTarget = ID3D11RenderTargetView;
using lagShaderResource = ID3D11ShaderResourceView;
using lagSampler = ID3D11SamplerState;

using lagTexture2D = ID3D11Texture2D;
using lagBuffer = ID3D11Buffer;
using lagInputAssembler = ID3D11InputLayout;
using lagPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY;
using lagRasterizerState = ID3D11RasterizerState;


class lagGraphics final {
public:
	static void Init(HWND Window) {
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
		Desc.OutputWindow = (HWND)Window;
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
			&sm_pSwapChain, &sm_pDevice, &ReturnLevel, &sm_pDeviceContext
		);
		if (FAILED(hr)) {
			OutputDebugStringA("[D3D11-INIT]: Failure in initializing D3D11. Cannot render!\\n");
			__debugbreak();
		}
		sm_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&sm_pTexture));
	}
	static void Shutdown() {
		sm_pTexture->Release();
		sm_pDevice->Release();
		sm_pDeviceContext->Release();
		sm_pSwapChain->Release();
	}
	static void SetRenderTargets(std::vector<lagRenderTarget*> pTargets, ID3D11DepthStencilView* DepthStencil) {
		sm_pDeviceContext->OMSetRenderTargets(pTargets.size(), pTargets.data(), DepthStencil);
	}
	static void ClearRenderTarget(lagRenderTarget* pTarget, float* Color) {
		sm_pDeviceContext->ClearRenderTargetView(pTarget, Color);
	}
	static void Present(unsigned long SyncInterval, unsigned long Flags) {
		sm_pSwapChain->Present(SyncInterval, Flags);
	}
	static lagBuffer* CreateBuffer(const D3D11_BUFFER_DESC* pBuffer, D3D11_SUBRESOURCE_DATA* pData) {
		lagBuffer* Buffer{nullptr};
		HRESULT hr = sm_pDevice->CreateBuffer(pBuffer, pData, &Buffer);
		if (FAILED(hr)) {
			return nullptr;
		}
		return Buffer;
	}
	static lagRasterizerState* CreateRasterizationState(const D3D11_RASTERIZER_DESC* pState) {
		lagRasterizerState* pRet{};
		HRESULT hr = sm_pDevice->CreateRasterizerState(pState, &pRet);
		if (FAILED(hr)) {
			return nullptr;
		}
		return pRet;
	}
	static lagVertexShader* CreateVertexShader(std::vector<char>& ByteCode, ID3D11ClassLinkage* Linkage = nullptr) {
		lagVertexShader* pVer{};
		HRESULT hr = sm_pDevice->CreateVertexShader(ByteCode.data(), ByteCode.size(), Linkage, &pVer);
		if (FAILED(hr)) {
			return nullptr;
		} 
		return pVer;
	}
	static lagFragmentShader* CreateFragmentShader(std::vector<char>& ByteCode, ID3D11ClassLinkage* Linkage = nullptr) {
		lagFragmentShader* pVer{};
		HRESULT hr = sm_pDevice->CreatePixelShader(ByteCode.data(), ByteCode.size(), Linkage, &pVer);
		if (FAILED(hr)) {
			return nullptr;
		}
		return pVer;
	}
	static lagInputAssembler* CreateInputAssembler(const std::vector<D3D11_INPUT_ELEMENT_DESC>& Descs, const std::vector<char>& ShaderByte) {
		lagInputAssembler* pAssembler{nullptr};
		HRESULT hr = sm_pDevice->CreateInputLayout(Descs.data(), Descs.size(), ShaderByte.data(), ShaderByte.size(), &pAssembler);
		if (FAILED(hr)) {
			return nullptr;
		}
		return pAssembler;
	}
	static lagRenderTarget* CreateRenderTarget(lagResource* Resource, const D3D11_RENDER_TARGET_VIEW_DESC* Desc = nullptr) {
		lagRenderTarget* Target{nullptr};
		HRESULT hr = sm_pDevice->CreateRenderTargetView(Resource, Desc, &Target);
		if (FAILED(hr)) {
			return nullptr;
		}
		return Target;
	}
	static lagTexture2D* CreateTexture2D(const D3D11_TEXTURE2D_DESC* Desc, D3D11_SUBRESOURCE_DATA* pData) {
		lagTexture2D* pTexture = nullptr;
		HRESULT hr = sm_pDevice->CreateTexture2D(Desc, pData, &pTexture);
		if (FAILED(hr)) {
			return nullptr;
		} 
		return pTexture;
	}
	static lagShaderResource* CreateShaderResource(lagResource* pRes, const D3D11_SHADER_RESOURCE_VIEW_DESC* Desc) {
		lagShaderResource* View{nullptr};
		HRESULT hr = sm_pDevice->CreateShaderResourceView(pRes, Desc, &View);
		if (FAILED(hr)) {
			return nullptr;
		}
		return View;
	}
	static lagResource* GetBackBuffer() {
		return sm_pTexture;
	}
public:
	static void Draw(UINT VertexCount, UINT VertexStart) {
		sm_pDeviceContext->Draw(VertexCount, VertexStart);
	}
	static void DrawIndexed(UINT IndexCount, UINT VertexStart, UINT VertexLocation) {
		sm_pDeviceContext->DrawIndexed(IndexCount, VertexStart, VertexLocation);
	}
	static void* Map(lagResource* Resource, UINT Subresource, D3D11_MAP MapType, UINT MapFlags) {
		D3D11_MAPPED_SUBRESOURCE Data{};
		sm_pDeviceContext->Map(Resource, Subresource, MapType, MapFlags, &Data);
		return Data.pData;
	}
	static void Unmap(lagResource* Resource, UINT subresource) {
		sm_pDeviceContext->Unmap(Resource, subresource);
	}
public:
	static void SetVertexShader(lagVertexShader* Shader) {
		if (sm_pCurrentVertexShader == Shader) return;
		sm_pDeviceContext->VSSetShader(Shader, nullptr, 0);
		sm_pCurrentVertexShader = Shader;
	}
	static void SetVertexShaderBuffers(UINT StartSlot, const std::vector<lagBuffer*>& buffers) {
		sm_pDeviceContext->VSSetConstantBuffers(StartSlot, buffers.size(), buffers.data());
	}
	static void SetFragmentShader(lagFragmentShader* Shader) {
		if (sm_pCurrentFragmentShader == Shader) return;
		sm_pDeviceContext->PSSetShader(Shader, nullptr, 0);
		sm_pCurrentFragmentShader = Shader;
	}
	static void SetFragmentShaderBuffers(UINT StartSlot, const std::vector<lagBuffer*>& buffers) {
		sm_pDeviceContext->PSSetConstantBuffers(StartSlot, buffers.size(), buffers.data());
	}
	static void SetFragmentShaderResources(UINT StartSlot, const std::vector<lagShaderResource*> Resources) {
		sm_pDeviceContext->PSSetShaderResources(StartSlot, Resources.size(), Resources.data());
	}
	static void SetFragmentShaderSamplers(UINT StartSlot, const std::vector<lagSampler*>& Sampler) {
		sm_pDeviceContext->PSSetSamplers(0, Sampler.size(), Sampler.data());
	}
	static void SetHullShader(lagHullShader* Shader) {
		if (sm_pCurrentHullShader == Shader) return;
		sm_pDeviceContext->HSSetShader(Shader, nullptr, 0);
		sm_pCurrentHullShader = Shader;
	}
	static void SetDomainShader(lagDomainShader* Shader) {
		if (sm_pCurrentDomainShader == Shader) return;
		sm_pDeviceContext->DSSetShader(Shader, nullptr, 0);
		sm_pCurrentDomainShader = Shader;
	}
	static void SetGeometryShader(lagGeometryShader* Shader) {
		if (sm_pCurrentGeometryShader == Shader) return;
		sm_pDeviceContext->GSSetShader(Shader, nullptr, 0);
		sm_pCurrentGeometryShader = Shader;
	}
	static void SetInputAssembler(lagInputAssembler* Assembler) {
		if (sm_pCurrentAssembler == Assembler)return;
		sm_pDeviceContext->IASetInputLayout(Assembler);
		sm_pCurrentAssembler = Assembler;
	}
	static void SetPrimitiveTopology(lagPrimitiveTopology Top) {
		if (sm_CurrentTopology == Top) return;
		sm_pDeviceContext->IASetPrimitiveTopology(Top);
		sm_CurrentTopology = Top;
	}
	static void SetRasterizerState(lagRasterizerState* Rasterizer) {
		if (sm_pCurrentRasterizerState == Rasterizer) return;
		sm_pDeviceContext->RSSetState(Rasterizer);
		sm_pCurrentRasterizerState = Rasterizer;
	}
	static void SetVertexBuffers(UINT StartSlot, const std::vector<lagBuffer*>& buffers, const UINT* Strides, const UINT* Offsets) {
		sm_pDeviceContext->IASetVertexBuffers(StartSlot, buffers.size(), buffers.data(), Strides, Offsets);
	}
	static void SetIndexBuffer(lagBuffer* Buffer, DXGI_FORMAT Format, UINT Offset) {
		if (sm_pCurrentIndexBuffer == Buffer) return;
		sm_pDeviceContext->IASetIndexBuffer(Buffer, Format, Offset);
		sm_pCurrentIndexBuffer = Buffer;
	}
	static void SetViewports(const std::vector<D3D11_VIEWPORT>& Ports) {
		sm_pDeviceContext->RSSetViewports(Ports.size(), Ports.data());
	}

private:
	lagGraphics() = delete;
	lagGraphics(const lagGraphics&) = delete;
	lagGraphics& operator=(const lagGraphics&) = delete;
	lagGraphics& operator=(lagGraphics&&) = delete;
	lagGraphics(lagGraphics&&) = delete;
	~lagGraphics() {}
private:
	static inline lagVertexShader* sm_pCurrentVertexShader = nullptr;
	static inline lagFragmentShader* sm_pCurrentFragmentShader = nullptr;
	static inline lagHullShader* sm_pCurrentHullShader = nullptr;
	static inline lagDomainShader* sm_pCurrentDomainShader = nullptr;
	static inline lagGeometryShader* sm_pCurrentGeometryShader = nullptr;
	static inline lagInputAssembler* sm_pCurrentAssembler = nullptr;
	static inline lagPrimitiveTopology sm_CurrentTopology = lagPrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	static inline lagRasterizerState* sm_pCurrentRasterizerState = nullptr;
	static inline lagBuffer* sm_pCurrentIndexBuffer = nullptr;
private:
	static inline lagResource* sm_pTexture{nullptr};
	static inline lagDeviceHandle* sm_pDevice{nullptr};
	static inline lagDeviceContext* sm_pDeviceContext{nullptr};
	static inline lagSwapChain* sm_pSwapChain{nullptr};
};
class CRenderPass_GameEntities {
public:
	CRenderPass_GameEntities(lagGraphics* pDevice) : m_pDevice(pDevice){

	}
	~CRenderPass_GameEntities() {
		m_pDevice = nullptr;
	}
private:
	lagGraphics* m_pDevice;
};
template<typename T, size_t Size> size_t ArraySize(const T(&Op)[Size]) {
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
		m_Desc.ByteWidth = ArraySize(quadVertices);
		D3D11_SUBRESOURCE_DATA Data{};
		Data.pSysMem = quadVertices;
		Buffer = lagGraphics::CreateBuffer(&m_Desc, &Data);


		lagShaderCompiler Comp{};

		lagByteCode Byte = Comp.Compile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\lag_basic.hlsl", "VS_Main", "vs_5_0");
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

		Byte = Comp.Compile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\lag_basic.hlsl", "PS_Main", "ps_5_0");
		PSShader = lagGraphics::CreateFragmentShader(Byte.Byte);
	}
	DirectX::XMMATRIX GetMatrix() const {
		return DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&m_Position)) * DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&m_Rotation)) * DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&m_Scale));
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
struct LAGMatrices {
	DirectX::XMMATRIX m_Projection;
	DirectX::XMMATRIX m_View;
	DirectX::XMMATRIX m_Model;
};
class CRenderer {
public:
	static void Init(HWND Window) {
		lagGraphics::Init(Window);
		sm_pRTV = lagGraphics::CreateRenderTarget(lagGraphics::GetBackBuffer());
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
		RasterDesc.CullMode = D3D11_CULL_BACK;
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
		lagGraphics::SetRenderTargets({sm_pRTV}, nullptr);

		D3D11_BUFFER_DESC BufferDesc{};
		BufferDesc.ByteWidth = sizeof(LAGMatrices);
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		LAGConstants = lagGraphics::CreateBuffer(&BufferDesc, nullptr);
		Quad = new CBillboard();
	}
	static inline lagBuffer* LAGConstants = nullptr;
	static inline CBillboard* Quad = nullptr;
	static void Process() {
		float f[4] = {0.5,0,0,1};
		lagGraphics::ClearRenderTarget(sm_pRTV, f);

		LAGMatrices Constants{};
		Constants.m_View = DirectX::XMMatrixLookAtLH({2,0,-5}, {0,0,0}, {0,1,0});
		Constants.m_Projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f), 16.0f / 9.0f, 0.01f, 100);
		Constants.m_Model = Quad->GetMatrix();

		Constants.m_Model = DirectX::XMMatrixTranspose(Constants.m_Model);
		Constants.m_View = DirectX::XMMatrixTranspose(Constants.m_View);
		Constants.m_Projection = DirectX::XMMatrixTranspose(Constants.m_Projection);

		LAGMatrices* mats = (LAGMatrices*)lagGraphics::Map(LAGConstants, 0, D3D11_MAP_WRITE_DISCARD, 0);
		memcpy(mats, &Constants, sizeof(LAGMatrices));
		lagGraphics::Unmap(LAGConstants, 0);
		lagGraphics::SetVertexShaderBuffers(0, {LAGConstants});

		Quad->Draw();
		lagGraphics::Present(1, 0);
	}
	static void Destroy() {
		lagGraphics::Shutdown();
	}
private:
	static inline lagRenderTarget* sm_pRTV{nullptr};
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	CTimer::Start();
	CWinArgs args = CWinArgs{hInstance, hPrevInstance, pCmdLine, nCmdShow};
	/* Anything past this point is APPLICATION specific. I.E it can vary how the applet wants to work. The framework can give them the CMD line stuff but whether or not they use it, up to them :)*/
	CWindowClass Class{{LE_Process, L"lagGraphics", hInstance}};
	const wchar_t* Name = {L"WindowName"};
	WindowDescription Desc{};
	Desc.WindowName = Name;
	CWindow Window{&Class, Desc};
	Window.Show();
	bool ShouldApplicationClose = false;
	CRenderer::Init(Window.GetHandle());
	while (!ShouldApplicationClose) {
		//CTimer::Tick();
		MSG m{};
		if (Window.Peek(m)) {
			TranslateMessage(&m);
			DispatchMessage(&m);
			if (m.message == WM_QUIT) {
				ShouldApplicationClose = true;
			}
		}
		CRenderer::Process();
	}
	CRenderer::Destroy();

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