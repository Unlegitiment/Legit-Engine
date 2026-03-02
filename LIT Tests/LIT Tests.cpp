#include <stdio.h>
#include <stdlib.h>
#include <LITemplates/alloc/Default.h>
/*
	This part below is what I hate about WS2. Weird headers.
*/
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
		netException(const char* Exception) : m_Message(Exception){

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

using namespace legit;
namespace dx {
#include <DirectXMath.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
}
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
void CreateWinClass(WNDPROC proc) {
	WNDCLASSW cls{};
	cls.hInstance = GetModuleHandle(NULL);
	cls.lpszClassName = L"lagWindow";
	cls.lpfnWndProc = proc;
	RegisterClassW(&cls);
}
static LRESULT CALLBACK WindowProcess(HWND Window, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CLOSE:
			PostQuitMessage(0);
	}

	return DefWindowProc(Window, msg, wParam, lParam);
}
static HWND WindowMain() {
	CreateWinClass(WindowProcess);

	HWND window = CreateWindowExW(NULL, L"lagWindow", L"WindowName", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);
	if (!window) {
		netLogger::Send("Window failed to create\n");
	}
	return window;
}
void WindowDef(bool& ShouldWindowClose) {
	MSG m{};
	while (PeekMessage(&m, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&m);
		DispatchMessageA(&m);
		if (m.message == WM_QUIT) {
			ShouldWindowClose = true;
		}
	}
}
#define SerializeFeatCase(lvl) case dx::D3D_FEATURE_LEVEL::lvl: return "dx::D3D_FEATURE_LEVEL::" #lvl;
const char* SerializeFeatureLevel(dx::D3D_FEATURE_LEVEL level) {
	using namespace dx;
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

dx::DXGI_SWAP_CHAIN_DESC CreateBasicDesc(HWND wnd) {
	dx::DXGI_SWAP_CHAIN_DESC SwapChainDesc{};
	SwapChainDesc.BufferDesc.Width = 0;
	SwapChainDesc.BufferDesc.Height = 0;
	SwapChainDesc.BufferDesc.Format = dx::DXGI_FORMAT_B8G8R8A8_UNORM;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
	SwapChainDesc.BufferDesc.Scaling = dx::DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.BufferDesc.ScanlineOrdering = dx::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = 1;
	SwapChainDesc.OutputWindow = wnd;
	SwapChainDesc.Windowed = true;
	SwapChainDesc.SwapEffect = dx::DXGI_SWAP_EFFECT_DISCARD;
	SwapChainDesc.Flags = 0;
	return SwapChainDesc;
}
#define gLog(fmt, ...) netLogger::Send({"[LEGraphics] " fmt, __VA_ARGS__})
static bool D3DLog(HRESULT FunctionCallReturn) {
	if (FAILED(FunctionCallReturn)) {
		gLog("D3D Function Failed. StopCode: %d\n", FunctionCallReturn);
		return false;
	}
	return true;
}
dx::ID3D11Buffer* CreateQuadVertices(dx::ID3D11Device* Device) {
	float quadVertices[] = {
		// positions 
		-1.0f, -1.0f, 0,
		-1.0f,  1.0f, 0,
		 1.0f, -1.0f, 0,
		-1.0f,  1.0f, 0,
		 1.0f,  1.0f, 0,
		 1.0f, -1.0f, 0
	};
	dx::D3D11_BUFFER_DESC Desc{};
	Desc.BindFlags = dx::D3D11_BIND_VERTEX_BUFFER;
	Desc.Usage = dx::D3D11_USAGE_IMMUTABLE;
	Desc.ByteWidth = sizeof(quadVertices);
	dx::D3D11_SUBRESOURCE_DATA Data{};
	Data.pSysMem = quadVertices;
	dx::ID3D11Buffer* Buffer{};
	HRESULT hr = Device->CreateBuffer(&Desc, &Data, &Buffer);
	if (FAILED(hr)) {
		gLog("Failed to create buffer (quadVertices)");
	}
	return Buffer;
}
/*
	I Feel Like I keep getting stuck right here. Its not because of the effects getting more advanced. Its because of the reprocussions going forward. 

	Once a scene graph is defined. And its able to be used in the local sense. It effectively becomes a game of numbers. Defining Geometry and their Effects. 

	Once I write the Scene Graph, in my head. There is no going back. There is only forwards. That is both Scary. But also intriguing. This works now. It renders a quad in 3D. 

	But once it gets to that golden point. Then. Then, it is real, tangible. With that, bye for now. Soon, real progression will be made. 

*/
int main() {
	netPlatConfig::InitClass();
	netLogger::Init();
	HWND wnd = WindowMain();
	if (wnd) {
		ShowWindow(wnd, SW_SHOW);

		bool ShouldWindowClose = false;

		auto SwapChainDesc = CreateBasicDesc(wnd);

		dx::ID3D11Device* Device{};
		dx::IDXGISwapChain* SwapChain{};
		dx::ID3D11DeviceContext* Context{};
		dx::D3D_FEATURE_LEVEL FeatureLevel{};
		HRESULT hr = dx::D3D11CreateDeviceAndSwapChain(NULL, dx::D3D_DRIVER_TYPE_HARDWARE, NULL, dx::D3D11_CREATE_DEVICE_DEBUG | dx::D3D11_CREATE_DEVICE_BGRA_SUPPORT, NULL, 0, D3D11_SDK_VERSION, &SwapChainDesc, &SwapChain, &Device, &FeatureLevel, &Context);
		if (FAILED(hr)) {
			gLog("Failed To Create Device! ReasonCode:%d.\n", hr);
		}
		gLog("Initialized D3D11 {ID3D11Device: 0x%p, ID3D11DeviceContext: 0x%p, IDXGISwapChain: 0x%p, FeatureLevel: %s(%d)}\n", Device, SwapChain, Context, SerializeFeatureLevel(FeatureLevel), (int)FeatureLevel);
		dx::ID3D11Texture2D* BackBufferRaw{};
		D3DLog(SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBufferRaw)));
		dx::ID3D11RenderTargetView* BackBufferRTV{};
		D3DLog(Device->CreateRenderTargetView(BackBufferRaw, NULL, &BackBufferRTV));
		Context->OMSetRenderTargets(1, &BackBufferRTV, nullptr);
		
		dx::ID3D11Buffer* buffer = CreateQuadVertices(Device);
		UINT stride = sizeof(float) * 3;
		UINT offset = 0;
		Context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
		
		dx::ID3D11VertexShader* VShader{};
		dx::ID3DBlob* pShader{}, * pErr{};

		hr = dx::D3DCompileFromFile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\lag_basic.hlsl", NULL, NULL, "vs_main3d", "vs_5_0", 0, 0, &pShader, &pErr);
		if (!pShader || FAILED(hr)) {
			gLog("Failed to produce viable VS Shader: ErrorCode: %d.\n ErrorCodes: { %s } ", hr, pErr->GetBufferPointer());
		}
		hr = Device->CreateVertexShader(pShader->GetBufferPointer(), pShader->GetBufferSize(), NULL, &VShader);
		if (FAILED(hr)) {
			gLog("Failed D3D Call CreateVertexShader ErrorCode: %d\n", hr);
		}
		if (pErr) {
			pErr->Release();
		}
		dx::ID3D11InputLayout* In{};
		dx::D3D11_INPUT_ELEMENT_DESC ElementDesc{};
		ElementDesc.Format = dx::DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
		ElementDesc.SemanticIndex = 0;
		ElementDesc.SemanticName = "POSITION";
		Device->CreateInputLayout(&ElementDesc, 1, pShader->GetBufferPointer(), pShader->GetBufferSize(), &In);
		if (pShader) {
			pShader->Release();
		}
		hr = dx::D3DCompileFromFile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\lag_basic.hlsl", NULL, NULL, "ps_main", "ps_5_0", 0, 0, &pShader, &pErr);
		if (!pShader || FAILED(hr)) {
			gLog("Failed to produce viable PS Shader: ErrorCode: %d.\n ErrorCodes: { %s } ", hr, pErr->GetBufferPointer());
		}
		dx::ID3D11PixelShader* PShader{};
		hr = Device->CreatePixelShader(pShader->GetBufferPointer(), pShader->GetBufferSize(), NULL, &PShader);
		if (FAILED(hr)) {
			gLog("Failed D3D Call CreatePixelShader ErrorCode: %d\n", hr);
		}
		Context->VSSetShader(VShader, nullptr, 0);
		Context->PSSetShader(PShader, nullptr, 0);
		Context->IASetInputLayout(In);
		Context->IASetPrimitiveTopology(dx::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		dx::D3D11_VIEWPORT vPort{};
		vPort.TopLeftX = 0;
		vPort.TopLeftY = 0;
		RECT re{};
		GetClientRect(wnd, &re);
		auto width = re.right - re.left;
		auto height = re.bottom - re.top;
		vPort.Height = height;
		vPort.Width = width;
		Context->RSSetViewports(1, &vPort);
		auto Model = dx::DirectX::XMMatrixTranspose(dx::DirectX::XMMatrixTranslation(0,0,0) * dx::DirectX::XMMatrixRotationRollPitchYaw(dx::DirectX::XMConvertToRadians(50), dx::DirectX::XMConvertToRadians(20), dx::DirectX::XMConvertToRadians(0)) * dx::DirectX::XMMatrixScaling(1, 1, 1));
		auto Projection = dx::DirectX::XMMatrixTranspose(dx::DirectX::XMMatrixPerspectiveFovLH(dx::DirectX::XMConvertToRadians(90.f), 16.0 / 9.0, 0.01f, 100));
		auto View = dx::DirectX::XMMatrixTranspose(dx::DirectX::XMMatrixLookAtLH({2,0,-2}, {0,0,0}, {0,1,0}));
		/*
		    matrix m_Projection; // 
			matrix m_View; // shifts universe to camera
			matrix m_Model; // looks  
		*/
		dx::DirectX::XMMATRIX Mats[3] = {Projection, View, Model};

		dx::ID3D11Buffer* CBuffer{};
		dx::D3D11_BUFFER_DESC CBufferDesc{};
		CBufferDesc.BindFlags = dx::D3D11_BIND_CONSTANT_BUFFER;
		CBufferDesc.ByteWidth = sizeof(dx::DirectX::XMMATRIX) * 3;
		CBufferDesc.Usage = dx::D3D11_USAGE_DYNAMIC;
		CBufferDesc.CPUAccessFlags = dx::D3D11_CPU_ACCESS_WRITE;
		dx::D3D11_SUBRESOURCE_DATA CBufferData{};
		CBufferData.pSysMem = Mats;
		Device->CreateBuffer(&CBufferDesc, &CBufferData, &CBuffer);
		Context->VSSetConstantBuffers(0, 1, &CBuffer);
		int i = 0;
		while (!ShouldWindowClose) {
			WindowDef(ShouldWindowClose);
			float fColor[4] = {0,0,0,1};
			Context->ClearRenderTargetView(BackBufferRTV, fColor);
			Context->Draw(6,0);
			


			SwapChain->Present(0, 0);
			Sleep(1);
		}
	}
	netLogger::Shutdown();
	netPlatConfig::ShutdownClass();
}