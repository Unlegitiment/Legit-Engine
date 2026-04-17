#include <stdio.h>
#include <stdlib.h>
#include <LITemplates/alloc/Default.h>
/*
	This part below is what I hate about WS2. Weird headers.
*/
#define NOMINMAX
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#pragma comment (lib, "WS2_32")
namespace legit {
	class litFormat {
	public:
		template<typename...T>
		litFormat(const char* fmt, T&&... args) {
			snprintf(m_Buff, sizeof(m_Buff), fmt, args...);
			m_StringSize = strnlen_s(m_Buff, 1028);
		}
		unsigned long long GetStringSize() const {
			return this->m_StringSize;
		}
		const char* GetBuffer() const {
			return this->m_Buff;
		}
		operator const char* () {
			return this->m_Buff;
		}
	private:
		unsigned long long m_StringSize = 0ll;
		char m_Buff[1028]; // could be a templated object btw!
	};
}
namespace legit {
	/*
		Purpose: Basic TCP Connection via WSA. A Simple Demo to go along with a standard option.
	*/
	class wsaLock {
	public:
		wsaLock(WORD Version, LPWSADATA data) {
			ErrorCode = WSAStartup(Version, data);
		}
		int GetError() const {
			return this->ErrorCode;
		}
		~wsaLock() {
			printf(__FUNCTION__"\n");
			WSACleanup();
		}
	private:
		int ErrorCode = 0;
	};
	class netPlatConfig {
	public:
		static void InitClass() {
			if (g_PlatformConfiguration) return;
			g_PlatformConfiguration = new netPlatConfig();
		}
		static bool IsInitialized() {
			return g_PlatformConfiguration != nullptr;
		}
		static wsaLock& GetWindowsLock() {
			return g_PlatformConfiguration->m_WSAInit;
		}
		static void ShutdownClass() {
			delete g_PlatformConfiguration;
		}
	private:
		static inline netPlatConfig* g_PlatformConfiguration = nullptr;
	private:
		netPlatConfig() : m_WSAInit(MAKEWORD(2, 2), &m_Data) {}
		netPlatConfig(const netPlatConfig&) = delete;
		netPlatConfig& operator=(const netPlatConfig&) = delete;
		// this is platform specific but currently NOT worth transitioning away into a separate class.
		// as also likely will require a way to access netPlatConfig earlier in the code later because the lifetime assumption is that
		/*
			netPlatConfig -> Application Based/Initialized On Startup Destroyed at Close.
			Other items prefixed with net -> Scope dependant on developer choice.
		*/
		WSAData m_Data{};
		wsaLock m_WSAInit;
	};
	class netAddrInfo {
	public:
		using AddressT = addrinfo;
		netAddrInfo(const char* IP, const char* Port, const AddressT* pHints) {
			m_Error = GetAddrInfoA(IP, Port, pHints, &m_pResult);
		}
		netAddrInfo(const netAddrInfo&) = delete;
		netAddrInfo& operator=(const netAddrInfo&) = delete;
		netAddrInfo(netAddrInfo&& other) noexcept : m_pResult(other.m_pResult) {
			other.m_pResult = nullptr;
		}
		netAddrInfo& operator=(netAddrInfo&& other) noexcept {
			if (this != &other) {
				if (m_pResult)
					FreeAddrInfoA(m_pResult);

				m_pResult = other.m_pResult;
				m_Error = other.m_Error;

				other.m_pResult = nullptr;
			}
			return *this;
		}
		int GetError() const {
			return m_Error;
		}
		bool Succeeded() const {
			return m_Error == 0;
		}
		AddressT* GetResult() const {
			return m_pResult;
		}
		~netAddrInfo() {
			printf(__FUNCTION__"\n");
			if (m_pResult)
				FreeAddrInfoA(m_pResult);
		}
	private:
		int m_Error = 0;
		AddressT* m_pResult;
	};
	class netSocket {
	public:
		using SocketT = SOCKET;
		netSocket(const netAddrInfo& addr) {
			m_NameLen = addr.GetResult()->ai_addrlen;
			memcpy(&m_pName, addr.GetResult()->ai_addr, m_NameLen);
			m_Socket = socket(addr.GetResult()->ai_family, addr.GetResult()->ai_socktype, addr.GetResult()->ai_protocol);
		}
		netSocket(const netSocket&) = delete;
		netSocket& operator=(const netSocket&) = delete;
		SocketT GetSocket() const {
			return this->m_Socket;
		}
		operator SocketT() {
			return this->m_Socket;
		}
		int Connect() const {
			return connect(this->m_Socket, reinterpret_cast<const sockaddr*>(&this->m_pName), this->m_NameLen);
		}
		int Send(const void* Data, unsigned long long Size, int flags = 0) const {
			return send(this->m_Socket, (const char*)Data, Size, flags);
		}
		int Recieve(char* Buffer, unsigned long Size, int flags) const {
			return recv(this->m_Socket, Buffer, Size, flags);
		}
		~netSocket() {
			printf(__FUNCTION__"\n");
			closesocket(m_Socket);
		}
	private:
		sockaddr_storage m_pName; // Resolved. ( just didn't know about the storage type!
		int m_NameLen;
		SocketT m_Socket;
	};
	static constexpr const char* LOCALHOST = "127.0.0.1";
	class netLoggerClient {
	public:
		netLoggerClient(const char* IP, const char* Port) : m_pSocket(netAddrInfo(IP, Port, &m_Hints)) {
			if (m_pSocket == INVALID_SOCKET) {
				return;
			}
			iResult = m_pSocket.Connect();
			while (iResult < 0) {
				iResult = m_pSocket.Connect();
			}
			if (m_pSocket == INVALID_SOCKET) {
				return;
			}
		}
		void Send(const litFormat& fmt) {
			int total = 0;
			int size = fmt.GetStringSize();

			while (total < size) {
				int sent = m_pSocket.Send(fmt.GetBuffer() + total, size - total);
				if (sent == SOCKET_ERROR)
					return;
				total += sent;
			}
		}
	private:
		static addrinfo TCPHints() {
			addrinfo hints;
			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_INET; // ipv4 or ipv6. Both function in a similar manner however. 
			hints.ai_socktype = SOCK_STREAM; // TCP Or UDP.
			hints.ai_protocol = IPPROTO_TCP; // variant. TCP Or UDP. Dependant on both AF_INET and Sock_Stream || Sock_UGRAM.
			return hints;
		}
		addrinfo m_Hints = TCPHints();
		int iResult = 0l;
		netSocket m_pSocket;
	};
	class netException {
	public:
		netException(const char* Exception) : m_Message(Exception) {

		}
		const char* Message() {
			return this->m_Message;
		}
	private:
		const char* m_Message = nullptr;
	};
	class netLogger {
	public:
		static void Init() {
			// this might not be relevent here. wsa is apart of WINDOW's initialization procedure. 
			// thus its kinda useless here. the only thing that WOULD make it worth it is being able to check against potentially a global for is WSA is initted (specifically inside of the netSocket or netAddrInfo)
			// but even then, it will just hard fail anyways soooooo.
			if (!netPlatConfig::IsInitialized()) {
				throw netException("Platform is not initialized");
			}
			sm_Client = new netLoggerClient(LOCALHOST, "27015");
		}
		static void Send(const litFormat& String) {
			sm_Client->Send(String);
		}
		static void Shutdown() {
			delete sm_Client;
		}
	private:
		static inline netLoggerClient* sm_Client;
	};
}
#define RBlack		"\033[30m"  // Black Text
#define RRed		"\033[31m"  // Red Text
#define RGreen		"\033[32m"  // Green Text
#define RYellow		"\033[33m"  // Yellow Text
#define RBlue		"\033[34m"  // Blue Text
#define RPurple		"\033[35m"  // Purple Text
#define RCyan		"\033[36m"  // Cyan Text

#define RIRed		"\033[41m"  // Inversed Red Text
#define RIGreen		"\033[42m\033[30m"  // Inversed Green Text
#define RIYellow	"\033[43m\033[30m"  // Inversed Yellow Text
#define RIBlue		"\033[44m"  // Inversed Blue Text
#define RIPurple	"\033[45m"  // Inversed Purple Text
#define RICyan		"\033[46m"  // Inversed Cyan Text
// Lighter Variants.
#define RHIBlack		"\033[90m"			//Black
#define RHIRed			"\033[91m"			//Red
#define RHIGreen		"\033[92m"			//Green
#define RHIYellow		"\033[93m"			//Yellow
#define RHIBlue			"\033[94m"			//Blue
#define RHIPurple		"\033[95m"			//Purple
#define RHICyan			"\033[96m"			//Cyan
#define RHIWhite		"\033[97m"			//White
#define RNorm		"\033[0m"		// Normal Text


#include <DirectXMath.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define SerializeFeatCase(lvl) case D3D_FEATURE_LEVEL::lvl: return "D3D_FEATURE_LEVEL::" #lvl;
const char* SerializeFeatureLevel(D3D_FEATURE_LEVEL level) {
	switch (level) {
		SerializeFeatCase(D3D_FEATURE_LEVEL_1_0_CORE);
		SerializeFeatCase(D3D_FEATURE_LEVEL_1_0_GENERIC);
		SerializeFeatCase(D3D_FEATURE_LEVEL_9_1);
		SerializeFeatCase(D3D_FEATURE_LEVEL_9_2);
		SerializeFeatCase(D3D_FEATURE_LEVEL_9_3);
		SerializeFeatCase(D3D_FEATURE_LEVEL_10_0);
		SerializeFeatCase(D3D_FEATURE_LEVEL_10_1);
		SerializeFeatCase(D3D_FEATURE_LEVEL_11_0);
		SerializeFeatCase(D3D_FEATURE_LEVEL_11_1);
		SerializeFeatCase(D3D_FEATURE_LEVEL_12_0);
		SerializeFeatCase(D3D_FEATURE_LEVEL_12_1);
		SerializeFeatCase(D3D_FEATURE_LEVEL_12_2);
		default:
			return "Invalid Feature Level";
	}
}
#undef SerializeFeatCase

DXGI_SWAP_CHAIN_DESC CreateBasicDesc(HWND wnd) {
	DXGI_SWAP_CHAIN_DESC SwapChainDesc{};
	SwapChainDesc.BufferDesc.Width = 0;
	SwapChainDesc.BufferDesc.Height = 0;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = 1;
	SwapChainDesc.OutputWindow = wnd;
	SwapChainDesc.Windowed = true;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	SwapChainDesc.Flags = 0;
	return SwapChainDesc;
}
ID3D11Buffer* CreateQuadVertices(ID3D11Device* Device) {
	float quadVertices[] = {
		// positions 
		-1.0f, -1.0f, 0,
		-1.0f,  1.0f, 0,
		 1.0f, -1.0f, 0,
		-1.0f,  1.0f, 0,
		 1.0f,  1.0f, 0,
		 1.0f, -1.0f, 0
	};
	D3D11_BUFFER_DESC Desc{};
	Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	Desc.Usage = D3D11_USAGE_IMMUTABLE;
	Desc.ByteWidth = sizeof(quadVertices);
	D3D11_SUBRESOURCE_DATA Data{};
	Data.pSysMem = quadVertices;
	ID3D11Buffer* Buffer{};
	HRESULT hr = Device->CreateBuffer(&Desc, &Data, &Buffer);
	if (FAILED(hr)) {
		return nullptr;
	}
	return Buffer;
}
namespace legit {
	using u8 = unsigned char;
	using u16 = unsigned short;
	using u32 = unsigned long;
	using u64 = unsigned long long;
	using s8 = signed char;
	using s16 = signed short;
	using s32 = signed long;
	using s64 = signed long long;
	using Int = int;
	using UInt = unsigned int;
	using SInt = signed int;
}
namespace legit {
	using Byte = unsigned char;
	using Word = unsigned short;
	using DWord = unsigned long;
	using QWord = unsigned long long;
}
namespace legit {
	using Char8 = char;
	using WChar = wchar_t; // Significantly platform dependant. Ranges from 2-4 bytes.
	using Char16 = char16_t;
	using Char32 = char32_t;
}
namespace legit {
	using UnknownPointer = void*;
}
#define Assertf(cond, fmt, ...) do {if(cond) {netLogger::Send({"[LE-Assert]: Condition: " #cond " " fmt, __VA_ARGS__}); __debugbreak();} } while(0);

using namespace legit;
static constexpr const char* PathToModel = "C:\\Users\\codyc\\OneDrive\\Docs from Gaming PC\\Documents\\TestModels\\nano.glb";
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
enum class ShowValue : legit::s32{
	SV_Hide = SW_HIDE,
	SV_Normal = SW_NORMAL,
	SV_ActivateMinimize = SW_SHOWMINIMIZED,
	SV_ActivateMaximized = SW_SHOWMAXIMIZED,
	SV_ShowNoActivate = SW_SHOWNOACTIVATE,
	SV_Show = SW_SHOW,
	SV_Minimize = SW_MINIMIZE,
	SV_ShowDefault = SW_SHOWDEFAULT
};
class CWindow {
public:
	CWindow() = default;
	CWindow(const legit::WChar* ClassName, const legit::WChar* TitleName, legit::DWord Style, legit::Int X, legit::Int Y, legit::Int Width, legit::Int Height, legit::UnknownPointer Instance) {
		m_WindowHandle = CreateWindowEx(0, ClassName, TitleName, Style, X, Y, Width, Height, nullptr, nullptr, (HINSTANCE)Instance, nullptr);
		if (!IsHandleValid()) {
			m_iWindowError = GetLastError();
		}
	}
	bool IsHandleValid() const {
		return m_WindowHandle;
	}
	CWindow(HWND& Window) {
		this->m_WindowHandle = Window;
		Window = nullptr;
	}
	CWindow(const CWindow&) = delete;
	CWindow& operator=(const CWindow&) = delete;
	CWindow& operator=(CWindow&& move) noexcept {
		if (this->IsHandleValid()) {
			this->Destroy();
		}
		this->m_WindowHandle = move.m_WindowHandle;
		move.m_WindowHandle = 0;
		return *this;
	}
	CWindow(CWindow&& move) noexcept : m_WindowHandle(move.m_WindowHandle){
		move.m_WindowHandle = 0;
	}

	operator HWND() {
		return this->m_WindowHandle;
	}
	HWND GetHandle() const {
		return this->m_WindowHandle;
	}
public:
	bool Show(ShowValue Value = ShowValue::SV_Normal) const {
		return ShowWindow(this->m_WindowHandle, (legit::s32)Value);
	}
	bool Maximize() const {
		return Show(ShowValue::SV_ActivateMaximized);
	}
	bool Minimize() const {
		return Show(ShowValue::SV_Minimize);
	}
	bool Hide() const {
		return Show(ShowValue::SV_Hide);
	}
	bool SetPosition(legit::s32 X, legit::s32 Y, legit::s32 Width, legit::s32 Height, legit::u32 uFlags) const {
		return SetWindowPos(this->m_WindowHandle, nullptr, X, Y, Width, Height, uFlags);
	}
	LRESULT MessageWindow(legit::u32 Message, WPARAM wParam, LPARAM lParam) const {
		return SendMessage(this->m_WindowHandle, Message, wParam, lParam);
	}
	bool IsVisible() const {
		return IsWindowVisible(this->m_WindowHandle);
	}
	bool IsFocused() const {
		return GetForegroundWindow() == m_WindowHandle;
	}
	bool IsMinimized() const {
		return IsIconic(this->m_WindowHandle);
	}
	bool IsMaximized() const {
		return IsZoomed(this->m_WindowHandle);
	}
	bool GetPlacement(WINDOWPLACEMENT& Out) const {
		return GetWindowPlacement(this->m_WindowHandle, &Out);
	}
	bool Destroy() const {
		return DestroyWindow(this->m_WindowHandle);
	}
	bool GetClientRect(RECT& Out) const {
		return ::GetClientRect(this->m_WindowHandle, &Out);
	}
	~CWindow() {
		Destroy();
		this->m_WindowHandle = nullptr;
		m_iWindowError = 0;
	}
private:
	HWND m_WindowHandle;
	int m_iWindowError = 0;
};
class CTheWindow {
public:
	static void Init() {
		sm_pWindowClass.lpszClassName = L"legitWindow";
		sm_pWindowClass.hInstance = GetModuleHandleW(NULL);
		sm_pWindowClass.lpfnWndProc = WindowProc;
		// I call this the "I don't wanna blind myself on app start" protocol.
		DefaultWindowBrush = CreateSolidBrush(RGB(0,0,0));
		sm_pWindowClass.hbrBackground = DefaultWindowBrush;
		RegisterClass(&sm_pWindowClass);
		sm_pWindow = CWindow(L"legitWindow", L"Fuzzy", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, (legit::UnknownPointer)sm_pWindowClass.hInstance);
		sm_pWindow.Show();
	}
	static void Update() {
		MSG m{};
		while (PeekMessage(&m, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&m);
			DispatchMessage(&m);
			if (m.message == WM_QUIT) {
				m_bShouldClose = true;
				netLogger::Send({"WM_QUIT\n"});
				break;
			}
		}
	}
	static bool ShouldClose() {
		return m_bShouldClose;
	}
	static CWindow& GetWindow() {
		return sm_pWindow;
	}
	static WNDCLASS& GetClass() {
		return sm_pWindowClass;
	}
	static void Shutdown() {
		DeleteObject(DefaultWindowBrush);
	}
private:
	static LRESULT CALLBACK WindowProc(HWND Window, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
			case WM_CLOSE:
				netLogger::Send({"WM_CLOSE\n"});
				DestroyWindow(Window);
				break;
			case WM_DESTROY:
				netLogger::Send({"WM_DESTROY\n"});
				PostQuitMessage(0);
				break;
			default:
				return DefWindowProc(Window, uMsg, wParam, lParam);
		}
	}
	static inline HBRUSH DefaultWindowBrush = 0;
	static inline bool m_bShouldClose = false;
	static inline WNDCLASS sm_pWindowClass{};
	static inline CWindow sm_pWindow{};
};
template<typename T> class sysComObject {
public:
	sysComObject() = default;
	sysComObject(T* Object) : Resource(Object){}
	sysComObject(const sysComObject<T>& Copy) : Resource(Copy.Resource) {
		Resource->AddRef();
	}
	sysComObject& operator=(const sysComObject<T>& Copy) {
		if (this->Resource) {
			this->Resource->Release();
		}
		this->Resource = Copy.Resource;
		this->Resource->AddRef();
		return *this;
	}
	sysComObject(sysComObject<T>&& move) noexcept : Resource(move.Resource){
		move.Resource = nullptr;
	}
	sysComObject& operator=(sysComObject<T>&& move) noexcept {
		if (this->Resource) {
			this->Resource->Release();
		}
		this->Resource = move.Resource;
		move.Resource = nullptr;
		return *this;
	}
	~sysComObject() {
		if (Resource) {
			Resource->Release();
		}
	}
	/*
		This is for complience with the Raw Com Object stuff. It's not entirely good practice to do a bunch of implicit casts, but it does improve convenience of sysComObject<T>.
	*/
	operator T* () const {
		return this->Resource;
	}
	/*
		We delete these because I don't want the case of ID3DBlob where inside of most constructors we don't take the raw arguments, but rather the void* equivalent which is a variable within the object. This results in potential issues and complications, thus. No void* casts without explicit intent.
	*/
	operator void*() = delete;
	/*
		This is considered unsafe, because in multi-threaded environments the reference count can change between calls.  
	*/
	legit::u32 GetReferenceCountUnsafe() const {
		Resource->AddRef();
		return Resource->Release();
	}
	static const IID& IdOf() {
		return __uuidof(T);
	}
	T* operator->() const { return Resource; }
	T& operator*() const {
		return *this->Resource;
	}
	T** operator&() {
		return &this->Resource;
	}
	sysComObject<T>* AddressOfThis() {
		return this;
	}
private:
	T* Resource = nullptr;
};
template<typename T> const IID& IdOf() {
	return __uuidof(T);
}
#define AssertHR(cond, fmt, ...) Assertf(FAILED(cond), fmt, __VA_ARGS__)

class CRenderer {
public:
	static constexpr legit::WChar ShaderPath[] = L"E:\\A_Development\\Legit Engine\\Main\\Project1\\lag_basic.hlsl";
	static constexpr char VSShaderMain[] = "vs_main";
	static constexpr char PSShaderMain[] = "ps_main";
	template<typename T> using DxObj = sysComObject<T>;
	CRenderer(CWindow& pWindow) {
		Assertf(!pWindow.IsHandleValid(), "Handle is not valid \n");
		DeviceSetup(pWindow);
		BackbufferSetup();
		m_QuadBuffer = CreateQuadVertices(this->m_pDevice);
		sysComObject<ID3DBlob> pBlob, pErr;
		AssertHR(D3DCompileFromFile(ShaderPath, nullptr, nullptr, VSShaderMain, "vs_5_0", 0,0, &pBlob, &pErr), "Failed to Compile Vertex Shader.\n");
		m_pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &this->m_VertexShader);
		D3D11_INPUT_ELEMENT_DESC ElementDesc{};
		ElementDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
		ElementDesc.SemanticIndex = 0;
		ElementDesc.SemanticName = "POSITION";
		AssertHR(m_pDevice->CreateInputLayout(&ElementDesc, 1, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &this->m_InputLayout), "Failed to generate Input Layout.\n");
		pBlob = nullptr;
		pErr = nullptr;
		AssertHR(D3DCompileFromFile(ShaderPath, nullptr, nullptr, PSShaderMain, "ps_5_0", 0,0,&pBlob, &pErr), "Failed to create Pixel Shader");
		AssertHR(m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &this->m_PixelShader), "Failed to create Pixel Shader");
		RECT r{};
		pWindow.GetClientRect(r);
		D3D11_VIEWPORT Viewport{};
		Viewport.TopLeftX = 0;
		Viewport.TopLeftY = 0;
		auto ClientWidth = (r.right - r.left);
		auto ClientHeight = r.bottom - r.top;
		Viewport.Height = ClientHeight;
		Viewport.Width = ClientWidth;
		legit::UInt Stride = 3 * sizeof(float);
		legit::UInt Offset = 0;
		m_pContext->RSSetViewports(1, &Viewport);
		m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pContext->IASetInputLayout(this->m_InputLayout);
		m_pContext->IASetVertexBuffers(0, 1, &this->m_QuadBuffer, &Stride, &Offset);
		m_pContext->VSSetShader(m_VertexShader, nullptr, 0);
		m_pContext->PSSetShader(m_PixelShader, nullptr, 0);
		m_pContext->OMSetRenderTargets(1, &this->m_BackBufferRTV, nullptr);
	}
	void Render() {
		FLOAT ColorClear[4] = {0.2,0,0,1};
		m_pContext->ClearRenderTargetView(this->m_BackBufferRTV, ColorClear);
		m_pContext->Draw(6, 0);
		m_pSwapChain->Present(1, 0); // I am gonna use VSync so that I don't explode my computer with a billion frames. tysm
	}
	CRenderer(const CRenderer&) = delete;
	CRenderer& operator=(const CRenderer&) = delete;
	CRenderer& operator=(CRenderer&&) = delete;
	CRenderer(CRenderer&&) = delete;
	~CRenderer() {
		
	}
private:
	DxObj<ID3D11InputLayout> m_InputLayout{};
	DxObj<ID3D11VertexShader> m_VertexShader{};
	DxObj<ID3D11PixelShader> m_PixelShader{};
	DxObj<ID3D11Buffer> m_QuadBuffer{};
	DxObj<ID3D11Texture2D> m_Backbuffer;
	DxObj<ID3D11RenderTargetView> m_BackBufferRTV;
	DxObj<ID3D11Device> m_pDevice;
	DxObj<ID3D11DeviceContext> m_pContext;
	DxObj<IDXGISwapChain> m_pSwapChain;
private:
	void DeviceSetup(CWindow& pWindow) {
		auto SwapChainDesc = CreateBasicDesc(pWindow); // dxgi's types should kindly consider getting cancer and dying a horrible death, cs wtf is with the 3 underscores, what is this an xbox360 gamertag, I thank god for 'auto' everyday.
		legit::UInt uFlags = 0;
		uFlags = D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		D3D_FEATURE_LEVEL ExpectedFeatureLevels[1]{
			D3D_FEATURE_LEVEL_11_0
		};
		IDXGISwapChain* Swap{};
		ID3D11Device* Dev{};
		ID3D11DeviceContext* Ctx{};
		D3D_FEATURE_LEVEL FeatureLevelAcquired;
		HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, uFlags, ExpectedFeatureLevels, 1, D3D11_SDK_VERSION, &SwapChainDesc, &Swap, &Dev, &FeatureLevelAcquired, &Ctx);
		Assertf(FAILED(hr), "D3D11 Device Setup as failed. Code: %d\n", hr);
		if (FeatureLevelAcquired < D3D_FEATURE_LEVEL_11_0) {
			netLogger::Send({"Feature Level is not up to par. Where we'd normally error handle or try to salvage it, I don't care. This isn't an API.\n"});
			__debugbreak();
		}
		this->m_pDevice = Dev;
		this->m_pContext = Ctx;
		this->m_pSwapChain = Swap;
	}
	void BackbufferSetup() {
		AssertHR(this->m_pSwapChain->GetBuffer(0, m_Backbuffer.IdOf(), (void**)&this->m_Backbuffer), "Failed to get backbuffer\n");
		AssertHR(this->m_pDevice->CreateRenderTargetView(this->m_Backbuffer, nullptr, &this->m_BackBufferRTV), "Couldn't Create RTV of BackBuffer\n");
	}
};

class CTheRenderer {
public:
	static void Init() {
		sm_pRenderer = new CRenderer(CTheWindow::GetWindow());
	}
	static void Update() {
		sm_pRenderer->Render();
	}
	static CRenderer* GetRendererInstance() {
		return sm_pRenderer;
	}
	static void Shutdown() {
		delete sm_pRenderer;
	}
private:
	static inline CRenderer* sm_pRenderer = nullptr;
};

int main() {
	netPlatConfig::InitClass();
	netLogger::Init();
	CTheWindow::Init();
	CTheRenderer::Init();
	while (!CTheWindow::ShouldClose()) {
		CTheWindow::Update();
		CTheRenderer::Update();
	}
	CTheRenderer::Shutdown();
	CTheWindow::Shutdown();
	netLogger::Shutdown();
	netPlatConfig::ShutdownClass();
	return 0;
}