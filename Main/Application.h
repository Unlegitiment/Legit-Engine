#pragma once
#define NOMINMAX
#include <LECore/core/args.h>
#include "WindowHandling/CWindow.h"
#include <iostream>
#include <LECore/maths/vec2.h>
#include <LECore/types.h>
#include <unordered_map>
#include <array>
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
struct lagShaderBytecode {
	lagShaderBytecode(void* pByte, size_t size){
		this->m_ByteCode.resize(size);
		memcpy(this->m_ByteCode.data(), pByte, size);
	}
	~lagShaderBytecode() {

	}
	const void* GetByte() const {
		return m_ByteCode.data();
	}
	size_t GetSize() const {
		return m_ByteCode.size();
	}
	std::vector<char> m_ByteCode;
};

class lagCompilerDX11 {
public:
	lagShaderBytecode Compile(const wchar_t* path, const char* entryPoint, const char* ShaderVersion, UINT flags) {
		ID3DBlob* ErrorBlob{},* Blob{};
		HRESULT hr;
		hr = D3DCompileFromFile(path, NULL, NULL, entryPoint, ShaderVersion, flags, 0, &Blob, &ErrorBlob); // This might be just static behavior. 
		if (FAILED(hr)) {
			std::cout << "[lagCompilerDX11] " << ErrorBlob->GetBufferPointer() << std::endl;
			ErrorBlob->Release();
			return{nullptr,0};
		}
		/* YOU JUST GENIUNELY CANNOT MAKE THIS SHIT UP LMAO PEAK GPT MOMENT HAHAHAHAHAHAHAHA. WHAT A GARBAGE FUCKING ROBOT LMAO.*/
		lagShaderBytecode byte = { Blob->GetBufferPointer(), Blob->GetBufferSize() };
		Blob->Release();
		return byte;
	}
};
class lagShaderCompiler {
public:
	static void InitClass() {
		sm_Compiler = new lagCompilerDX11();
	}
	static lagShaderBytecode Compile(const wchar_t* path, const char* entryPoint, const char* ShaderVersion, UINT flags) {
		return sm_Compiler->Compile(path, entryPoint, ShaderVersion, flags);
	}
	static void DestroyClass() {
		delete sm_Compiler;
	}
private:
	static inline lagCompilerDX11* sm_Compiler;
};

using DeviceSignature = ID3D11Device * (*)();
static DeviceSignature GetDevice;
//If blobs are not useful past the compile stage and the Input Assembler might scrap the in-class definition for compiling shaders and put it into a different object. 
class lagVertexShader {
public:
	static constexpr const char* SHADER_DBG_FAIL = "[VS] ";
	lagVertexShader(const lagShaderBytecode& byteCode) {
		HRESULT hr;
		hr = GetDevice()->CreateVertexShader(byteCode.GetByte(), byteCode.GetSize(), nullptr, &m_ShaderProgram);
		if (FAILED(hr)) {
			std::cout << SHADER_DBG_FAIL << hr << "\n\0";
			return;
		}
	}
	ID3D11VertexShader* GetShader() {
		return this->m_ShaderProgram;
	}
	~lagVertexShader() {
		if (m_ShaderProgram) m_ShaderProgram->Release();
	}
private:
	ID3D11VertexShader* m_ShaderProgram = nullptr;
};
class lagFragmentShader {
public:
	static constexpr const char* SHADER_DBG_FAIL = "[PS] ";
	lagFragmentShader(const lagShaderBytecode& byteCode) {
		HRESULT hr{};
		hr = GetDevice()->CreatePixelShader(byteCode.GetByte(), byteCode.GetSize(), nullptr, &m_ShaderProgram);
		if (FAILED(hr)) {
			std::cout << SHADER_DBG_FAIL << hr << std::endl;
			return;
		}
	}
	lagFragmentShader(const lagFragmentShader&) = delete;
	lagFragmentShader& operator=(const lagFragmentShader&) = delete;

	lagFragmentShader(lagFragmentShader&& rhs) noexcept {
		this->m_ShaderProgram = rhs.m_ShaderProgram;
		rhs.m_ShaderProgram = nullptr;
	}
	lagFragmentShader& operator=(lagFragmentShader&& rhs) noexcept {
		this->m_ShaderProgram = rhs.m_ShaderProgram;
		rhs.m_ShaderProgram = nullptr;
	}
	ID3D11PixelShader* GetShader() {
		return this->m_ShaderProgram;
	}
	~lagFragmentShader() {
		if(m_ShaderProgram) m_ShaderProgram->Release();
	}
private:
	ID3D11PixelShader* m_ShaderProgram = nullptr;
};

class lagInputAssembler {
public:
	lagInputAssembler(const D3D11_INPUT_ELEMENT_DESC* pPtr, size_t Size, const lagShaderBytecode& VS) {
		HRESULT hr{};
		hr = GetDevice()->CreateInputLayout(pPtr, Size, VS.GetByte(), VS.GetSize(), &m_Layout); 
		if (FAILED(hr)) {
			std::cout << hr << std::endl;
			return;
		}
	}
	lagInputAssembler(const lagShaderBytecode& VS) {
		HRESULT hr;
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
		element2.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		element2.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		element2.InstanceDataStepRate = 0;

		D3D11_INPUT_ELEMENT_DESC Desc[2] = { element1, element2 };
		hr = GetDevice()->CreateInputLayout(Desc, 2, VS.GetByte(), VS.GetSize(), &m_Layout); // 
		if (FAILED(hr)) {
			std::cout << hr << std::endl;
			return;
		}
	}
	ID3D11InputLayout* GetLayout() {
		return m_Layout;
	}
	~lagInputAssembler() {
		if (m_Layout) m_Layout->Release();
	}
private:
	ID3D11InputLayout* m_Layout;
};

struct Vertex {
	DirectX::XMFLOAT3 m_Positions;
	DirectX::XMFLOAT4 m_Color;
};
enum class eVertexType {
	VECTOR3,
	VECTOR4,
	MATRIX4X4,
	EVT_MAX
};
template<eVertexType V> struct VertexBase {
	static constexpr eVertexType m_VertexType = V;
};
template<typename T, DXGI_FORMAT F> struct VertexOfType {
	using Type = T;
	static constexpr int SizeOfType = sizeof (T);
	static constexpr DXGI_FORMAT m_TypeFormat = F;
};
template<eVertexType T> struct VertexEvaluater{
	using Type = void;
	static constexpr VertexOfType<Type, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN> VertexInformation;
};
template<> struct VertexEvaluater<eVertexType::VECTOR3> {
	using Type = DirectX::XMFLOAT3; 
	static constexpr VertexOfType<Type, DXGI_FORMAT_R32G32B32_FLOAT> VertexInformation;
};
template<> struct VertexEvaluater<eVertexType::VECTOR4> {
	using Type = DirectX::XMFLOAT4;
	static constexpr VertexOfType<Type, DXGI_FORMAT_R32G32B32A32_FLOAT> VertexInformation;
};
enum class eVertexNames {
	POSITION,
	COLOR,
};
template<eVertexNames PRIM> struct VrtxToStr {
	static constexpr const char* NAMES[] = {
		"POSITION",
		"COLOR",
	};
	static constexpr const char* GetName() {
		return NAMES[(int)PRIM];
	}
};
template<eVertexNames NAME, eVertexType T, int Index>
struct VertexDeclaration {
	static constexpr const char* ShaderName = VrtxToStr<NAME>::GetName(); // this fetches the name
	static const eVertexType Type = T;
	static constexpr int SemanicIndex = Index;
};
template<typename T> struct VertexFormatConversion {
	static constexpr D3D11_INPUT_ELEMENT_DESC GetDescription(D3D11_INPUT_CLASSIFICATION InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA) {
		D3D11_INPUT_ELEMENT_DESC Description{};
		Description.SemanticName = T::ShaderName; // Attribute
		Description.SemanticIndex = T::SemanicIndex;
		Description.Format = VertexEvaluater<T::Type>::VertexInformation.m_TypeFormat; // get this from evaluator.
		Description.InputSlot = 0; 
		Description.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		Description.InputSlotClass = InputSlotClass;
		Description.InstanceDataStepRate = 0;
		return Description;
	}
};
template<typename... V> struct VertexFormat {
	static lagInputAssembler* CreateLayout(const lagShaderBytecode& byteCode) {
		constexpr std::array<D3D11_INPUT_ELEMENT_DESC, sizeof...(V)> arr = {
			VertexFormatConversion<V>::GetDescription()...
		};
		lagInputAssembler* m_InputLayout = new lagInputAssembler(arr.data(), arr.size(), byteCode);
		return m_InputLayout;
	}
};
/*
	VertexFormat<{"POSITION", eVertexType::VECTOR3}, {"COLOR", eVertexType::VECTOR4}> m_VertexFormat;
*/
class lagVertexBuffer {
public:
	lagVertexBuffer(const void* pData, size_t iDataSize, size_t iStride) {
		D3D11_BUFFER_DESC Description{};
		Description.ByteWidth = iDataSize * iStride; // This is this size of my buffer?
		Description.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		Description.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
		Description.CPUAccessFlags = 0;
		Description.StructureByteStride = 0; // Im not sure how to use this?
		D3D11_SUBRESOURCE_DATA mData{};
		// Check this call. 
		mData.pSysMem = pData; // because internally I would assume that it copies the data TO the gpu. 
		HRESULT hr;
		hr = GetDevice()->CreateBuffer(&Description, &mData, &m_VertexBuffer);
		this->m_VertexSize = iDataSize; // Amount of Vertices to Draw.
	}
	ID3D11Buffer* GetBuffer() {
		return this->m_VertexBuffer;
	}
	ID3D11Buffer** GetBufferPtr() {
		return &this->m_VertexBuffer;
	}
	size_t GetSize() const {
		return m_VertexSize;
	}
	~lagVertexBuffer() {
		m_VertexBuffer->Release(); 
	}
private:
	size_t m_VertexSize;
	ID3D11Buffer* m_VertexBuffer;
};
class lagIndexBuffer {
public:
	lagIndexBuffer(const unsigned int* Buffer, size_t iDataSize) {
		D3D11_BUFFER_DESC Description{};
		Description.ByteWidth = iDataSize * sizeof (unsigned int); // This is this size of my buffer?
		Description.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		Description.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
		Description.CPUAccessFlags = 0;
		Description.StructureByteStride = 0; // Im not sure how to use this?
		D3D11_SUBRESOURCE_DATA mData{};
		mData.pSysMem = Buffer; // because internally I would assume that it copies the data TO the gpu. 
		
		HRESULT hr;
		hr = GetDevice()->CreateBuffer(&Description, &mData, &m_Buffer);
		this->m_iDataSize = iDataSize;
	}
	ID3D11Buffer* GetBuffer() {
		return m_Buffer;
	}
	ID3D11Buffer** GetBufferPtr() {
		return &m_Buffer;
	}
	size_t GetSize() const {
		return m_iDataSize;
	}
	~lagIndexBuffer() {
		if(m_Buffer) m_Buffer->Release();
	}
private:
	ID3D11Buffer* m_Buffer = nullptr;
	size_t m_iDataSize = 0;
};
class CMyCube {
public:
	CMyCube() {
		auto m_CPUVb = MakeModel();
		this->m_VB = new lagVertexBuffer(m_CPUVb.data(), m_CPUVb.size(), sizeof Vertex); // Type.
		auto cpuIb = MakeIndices();
		this->m_IB = new lagIndexBuffer(cpuIb.data(), cpuIb.size());
		m_LocalMtx = DirectX::XMMatrixTranslation(0, 5, 0);
		InitVertexInformation();
	}
	void InitVertexInformation() {
		lagShaderBytecode VS = lagShaderCompiler::Compile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\vs.hlsl", "main", "vs_5_0", D3DCOMPILE_DEBUG);
		m_VertexShader = new lagVertexShader(VS);
		lagShaderBytecode PS = lagShaderCompiler::Compile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\ps.hlsl", "Main", "ps_5_0", D3DCOMPILE_DEBUG);
		m_FragShader = new lagFragmentShader(PS);
		m_InputAssembler = m_Type.CreateLayout(VS); // This might be weird. 
	}
	DirectX::XMMATRIX& GetMtx() {
		return m_LocalMtx;
	}
	void Draw(ID3D11DeviceContext* pContext) {
		const UINT stride = sizeof Vertex;
		const UINT offset = 0;

		pContext->IASetInputLayout(m_InputAssembler->GetLayout());
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // I would assume.
		pContext->PSSetShader(this->m_FragShader->GetShader(), nullptr, 0); // Signature Similar?
		pContext->VSSetShader(this->m_VertexShader->GetShader(), nullptr, 0);
		pContext->IASetVertexBuffers(0,1,this->m_VB->GetBufferPtr(), &stride, &offset);
		pContext->IASetIndexBuffer(this->m_IB->GetBuffer(), DXGI_FORMAT_R32_UINT, 0); // I would assume thats the type lmao.
		pContext->DrawIndexed(m_IB->GetSize(), 0, 0); // test.
	}
	~CMyCube() {
		delete m_VertexShader;
		delete m_FragShader;
		delete m_InputAssembler;
		delete m_IB;
		delete m_VB;
	}
private:
	DirectX::XMMATRIX m_LocalMtx = DirectX::XMMatrixIdentity();
	std::vector<Vertex> MakeModel() {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile("C:\\Users\\codyc\\OneDrive\\Docs from Gaming PC\\Documents\\TestModels\\object.obj", aiPostProcessSteps::aiProcess_Triangulate);
		if (scene->HasMeshes()) {
			aiMesh* mesh = scene->mMeshes[0]; // Of course this is a static check because I already know how many models I have. Ideally I'd check this. 
			std::vector<Vertex> Mesh;
			for (int i = 0; i < mesh->mNumVertices; i++) {
				aiVector3D v3D = mesh->mVertices[i];
				Vertex v{};
				v.m_Positions = { v3D.x, v3D.y, v3D.z };

				if (i % 3) {
					v.m_Color = { 1.0,0,0,1.0 };
				}
				else if (i % 2) {
					v.m_Color = { 0,1.0,0,1.0 };
				}
				else if (i % 1) {
					v.m_Color = { 0,0,1.0,1.0 };
				}
				else {
					v.m_Color = { 1.0,1.0,1.0,1.0 };
				}
				Mesh.push_back(v);
			}
			return Mesh;
		}
		std::cout << "Scene does not have a mesh!\n\0";
		return { };
	}
	std::vector<unsigned int> MakeIndices() {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile("C:\\Users\\codyc\\OneDrive\\Docs from Gaming PC\\Documents\\TestModels\\object.obj", aiPostProcessSteps::aiProcess_Triangulate);
		if (scene->HasMeshes()) {
			aiMesh* mesh = scene->mMeshes[0]; // Of course this is a static check because I already know how many models I have. Ideally I'd check this. 
			std::vector<unsigned int> Mesh;
			if (mesh->HasFaces()) {
				for (int i = 0; i < mesh->mNumFaces; i++) {
					aiFace& Face = mesh->mFaces[i];
					Mesh.push_back(Face.mIndices[0]);
					Mesh.push_back(Face.mIndices[1]);
					Mesh.push_back(Face.mIndices[2]);
				}
			}
			return Mesh;
		}
		std::cout << "Scene does not have a mesh!\n\0";
		return { };
	}

	lagVertexShader* m_VertexShader = nullptr;
	lagFragmentShader* m_FragShader = nullptr;
	VertexFormat<
		VertexDeclaration<eVertexNames::POSITION, eVertexType::VECTOR3, 0>,
		VertexDeclaration<eVertexNames::COLOR, eVertexType::VECTOR4, 0>
	> m_Type;
	lagInputAssembler* m_InputAssembler = nullptr;
	lagVertexBuffer* m_VB = nullptr;
	lagIndexBuffer* m_IB = nullptr;
};
struct Constant {
	DirectX::XMMATRIX m_Projection;
	DirectX::XMMATRIX m_View;
	DirectX::XMMATRIX m_Model;
};
class camCamera{
public:
	camCamera() {

	}
	void SetPosition(DirectX::XMFLOAT3 _newPos) {
		m_Position = _newPos;
	}
	DirectX::XMMATRIX WhatWeLookingAt() const { 
		return DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&m_Position), DirectX::XMLoadFloat3(&m_Target), DirectX::XMLoadFloat3(&m_Up));
	}
	DirectX::XMMATRIX GetPerspectiveMtx(float fAspectRatio) {
		return DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(this->m_fFOV), fAspectRatio, m_fNearZ, m_fFarZ);
	}
	~camCamera() {

	}
public:
	float m_fFOV = 60.0; // 60 is a good default Although should just make it project spec :/
	float m_fNearZ = 0.00001f;
	float m_fFarZ = 100.f;
	DirectX::XMFLOAT3 m_Position = {0,0,-10}; // Target and Position can never meet. 
	DirectX::XMFLOAT3 m_Target = { 0,0,0 };
	DirectX::XMFLOAT3 m_Up = { 0,1,0 }; // Although we should change this to not be this. As up might be a global?
};
class CRenderer {
public:
	static CRenderer& Get() {
		static CRenderer s_Renderer;
		return s_Renderer;
	}
	static ID3D11Device* __GetDevice() {
		return Get().m_pDevice;
	}
	void InitDeviceLayer() {
		GetDevice = __GetDevice;
		lagShaderCompiler::InitClass();
	}

	void SetupConstantBuffer() {
		D3D11_BUFFER_DESC Desc{};
		Desc.Usage = D3D11_USAGE_DYNAMIC; // DYNAMIC BUFFER
		Desc.ByteWidth = sizeof(Constant);
		Desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		Desc.StructureByteStride = 0;
		Desc.MiscFlags = 0; // We do not have misc flags. 
		HRESULT hr;
		hr = m_pDevice->CreateBuffer(&Desc, nullptr, &m_ConstantBuffer);
		if (FAILED(hr)) {
			std::cout << "Buffer could not make constant buffer. " << hr << "\n\0";
			//CApplication::Get().GetWindow()->g_Close = true;
			return;
		}
	}
	void UpdateCameraPosition() { // Probably add an ImGui module to this or just make it dependent on the KB but because you don't have that abstraction yet, I recommend the ImGui route. . 
		
	}
	void Init(const CWindow* wind) {
		auto devs = sOutDevice::Init(wind->GetWindowHandle());
		this->m_pSwapChain = devs->m_pSwapChain;
		this->m_pDevice = devs->m_pDevice;
		this->m_pContext = devs->m_pContext;
		delete devs; // devs does not exist anymore. 
		InitDeviceLayer();
		m_MyCube = new CMyCube();
		m_Camera = new camCamera();

		m_Camera->m_fFarZ = 800.0f; 

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

		D3D11_RASTERIZER_DESC rastDesc{};
		rastDesc.FillMode = D3D11_FILL_SOLID;
		rastDesc.CullMode = D3D11_CULL_BACK; // Disable culling to avoid orientation issues
		rastDesc.DepthClipEnable = TRUE;

		ID3D11RasterizerState* rastState = nullptr;
		hr = m_pDevice->CreateRasterizerState(&rastDesc, &rastState);
		m_pContext->RSSetState(rastState);
		SetupConstantBuffer();
		if (FAILED(hr)) {
			std::cout << "[BUFFER]" << hr << std::endl;
			return;
		}
		CompileShaders();
	} 

	void CompileShaders() {

	}
	
	void Update() {

		UpdateCameraPosition();
		const float m_fClear[4] = { 0.145,0.145,0.16,1.0f };
		m_pContext->ClearRenderTargetView(m_View, m_fClear);
		
		D3D11_MAPPED_SUBRESOURCE Res{};
		m_pContext->Map(m_ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Res);

		Constant constant{};
		constant.m_Model = m_MyCube->GetMtx(); // This is based on the Object.
		constant.m_View = m_Camera->WhatWeLookingAt(); // This is based on the Camera
		float fAspect = viewport.Width / viewport.Height; 
		constant.m_Projection = m_Camera->GetPerspectiveMtx(fAspect); // This is based on the Camera && Display.
		constant.m_Model = XMMatrixTranspose(constant.m_Model);
		constant.m_View = XMMatrixTranspose(constant.m_View);
		constant.m_Projection = XMMatrixTranspose(constant.m_Projection);
		memcpy(Res.pData, &constant, sizeof(Constant));

		m_pContext->Unmap(m_ConstantBuffer, 0); // Subresource could be wrong.
		m_pContext->VSSetConstantBuffers(0, 1, &m_ConstantBuffer);

		m_MyCube->Draw(m_pContext);

		m_pSwapChain->Present(1, 0);
	}
	void Shutdown() {
		lagShaderCompiler::DestroyClass();
		delete m_MyCube;
		delete m_Camera;
		if(m_View) m_View->Release();
		if(m_pDevice) m_pDevice->Release();
		if(m_pSwapChain) m_pSwapChain->Release();
		if(m_pContext) m_pContext->Release();
	}
private:
	ID3D11Buffer* m_ConstantBuffer;
	camCamera* m_Camera = nullptr; // this is not entirely relevent to the renderer. 


	CMyCube* m_MyCube = nullptr;

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