#include <sstream>
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

using namespace legit;
namespace dx {
#include <DirectXMath.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
}
#include <vector>

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
class lagVertexShader {
public:
	lagVertexShader(dx::ID3D11Device* Device, const char* ByteCode, unsigned long long ByteCodeSize) {
		Device->CreateVertexShader(ByteCode, ByteCodeSize, nullptr, &m_VertexShader);
	}
	lagVertexShader(const lagVertexShader& copy) {
		this->m_ByteCode = copy.m_ByteCode;
		this->ByteSize = copy.ByteSize;
		this->m_VertexShader = copy.m_VertexShader;
	}
	~lagVertexShader() {
		m_VertexShader->Release();
	}
private:
	char* m_ByteCode{nullptr};
	unsigned long long ByteSize{};
	dx::ID3D11VertexShader* m_VertexShader{};
};
namespace legit {
	using u8 = unsigned char;
	using u16 = unsigned short;
	using u32 = unsigned long;
	using u64 = unsigned long long;
	using s8 = signed char;
	using s16 = signed short;
	using s32 = signed long;
	using s64 = signed long long;
}

class lagGeometry {
public:
	dx::DirectX::XMMATRIX World{};
	lagGeometry() = default;
	lagGeometry(dx::ID3D11Device* pDevice, float* Floats, legit::u64 VSize, legit::u32* Indices, legit::u64 ISize, legit::u32 Stride) : m_Stride(Stride), m_IndexCount(ISize), m_VertexCount(VSize / Stride) {
		//Vertex
		dx::D3D11_BUFFER_DESC VDesc{};
		VDesc.BindFlags = dx::D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
		VDesc.ByteWidth = VSize;
		VDesc.CPUAccessFlags = 0;
		VDesc.Usage = dx::D3D11_USAGE_IMMUTABLE;
		dx::D3D11_SUBRESOURCE_DATA VData{};
		VData.pSysMem = Floats;
		pDevice->CreateBuffer(&VDesc, &VData, &m_VertexBuffer); // not checking hresult!

		if (!Indices && ISize == 0) return; // early return warning?
		//Index
		VDesc.BindFlags = dx::D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
		VDesc.ByteWidth = ISize * sizeof(legit::u32);
		VDesc.CPUAccessFlags = 0;
		VDesc.Usage = dx::D3D11_USAGE_IMMUTABLE;
		VData.pSysMem = Indices;
		pDevice->CreateBuffer(&VDesc, &VData, &m_IndexBuffer); // not checking hresult!
	}
	lagGeometry(const lagGeometry& geo) : m_VertexBuffer(geo.m_VertexBuffer), m_IndexBuffer(geo.m_IndexBuffer) {
		geo.m_VertexBuffer->AddRef();
		geo.m_IndexBuffer->AddRef(); // Add + 1 to known ref.
	}
	lagGeometry& operator=(const lagGeometry& geo) {
		if (m_VertexBuffer) {
			m_VertexBuffer->Release();
		}
		if (m_IndexBuffer) {
			m_IndexBuffer->Release();
		}
		m_IndexBuffer = geo.m_IndexBuffer;
		m_VertexBuffer = geo.m_VertexBuffer;
		m_VertexBuffer->AddRef();
		m_IndexBuffer->AddRef();
		return *this;
	}
	lagGeometry(lagGeometry&& geo) noexcept : m_VertexBuffer(geo.m_VertexBuffer), m_IndexBuffer(geo.m_IndexBuffer), m_Stride(geo.m_Stride), m_IndexCount(geo.m_IndexCount), m_VertexCount(geo.m_VertexCount) {
		geo.m_IndexBuffer = nullptr;
		geo.m_VertexBuffer = nullptr;
	}
	lagGeometry& operator=(lagGeometry&& geo) noexcept {
		if (m_VertexBuffer) {
			m_VertexBuffer->Release();
		}
		if (m_IndexBuffer) {
			m_IndexBuffer->Release();
		}
		m_VertexBuffer = geo.m_VertexBuffer;
		m_IndexBuffer = geo.m_IndexBuffer;
		geo.m_IndexBuffer = nullptr;
		geo.m_VertexBuffer = nullptr;

		this->m_Stride = geo.m_Stride;
		this->m_IndexCount = geo.m_IndexCount;
		this->m_VertexCount = geo.m_VertexCount;
		return *(this);
	}
	~lagGeometry() {
		if (m_VertexBuffer)
			m_VertexBuffer->Release();
		if (m_IndexBuffer)
			m_IndexBuffer->Release();
	}
	void Draw(dx::ID3D11DeviceContext* pContext) const {
		UINT strLocal = m_Stride;
		UINT offset = 0;
		if (m_VertexBuffer)
			pContext->IASetVertexBuffers(0, 1, &m_VertexBuffer, &strLocal, &offset);
		if (m_IndexBuffer)
			pContext->IASetIndexBuffer(m_IndexBuffer, dx::DXGI_FORMAT_R32_UINT, 0);
		if (m_IndexBuffer)
			pContext->DrawIndexed(m_IndexCount, 0, 0);
		else {
			pContext->Draw(m_VertexCount, 0);
		}
	}
private:
	dx::ID3D11Buffer* m_VertexBuffer = nullptr;
	dx::ID3D11Buffer* m_IndexBuffer = nullptr;
	legit::u32 m_Stride = 0;
	legit::u32 m_IndexCount = 0;
	legit::u32 m_VertexCount = 0;
};
static void CreateQuadVerts(dx::ID3D11Device* pDev, lagGeometry& geo) {
	float quadVertices[] = {
		// positions 
		-1.0f, -1.0f, 0,
		-1.0f,  1.0f, 0,
		 1.0f, -1.0f, 0,
		-1.0f,  1.0f, 0,
		 1.0f,  1.0f, 0,
		 1.0f, -1.0f, 0
	};
	geo = lagGeometry(pDev, quadVertices, 18 * sizeof(float), nullptr, 0, sizeof(float) * 3);
}

/*
	I Feel Like I keep getting stuck right here. Its not because of the effects getting more advanced. Its because of the reprocussions going forward.

	Once a scene graph is defined. And its able to be used in the local sense. It effectively becomes a game of numbers. Defining Geometry and their Effects.

	Once I write the Scene Graph, in my head. There is no going back. There is only forwards. That is both Scary. But also intriguing. This works now. It renders a quad in 3D.

	But once it gets to that golden point. Then. Then, it is real, tangible. With that, bye for now. Soon, real progression will be made.
*/
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
class lagModel {
public:
	struct Mats {
		dx::DirectX::XMMATRIX Projection;
		dx::DirectX::XMMATRIX View;
		dx::DirectX::XMMATRIX Model;
	} g_Mats{};
	static constexpr const char* PathToModel = "C:\\Users\\codyc\\OneDrive\\Docs from Gaming PC\\Documents\\TestModels\\nano.glb";
	lagModel(dx::ID3D11Device* pDev, float fAspect = 16./9.) {
		Assimp::Importer Import{};
		const aiScene* pScene = Import.ReadFile(PathToModel, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ConvertToLeftHanded);
		if (!pScene || !pScene->mRootNode) {
			gLog("Failed to produce proper Scene.\n");
			gLog("Assimp Err: %s.\n", Import.GetErrorString());
			return;
		}
		Meshes.resize(pScene->mNumMeshes);
		for (int i = 0; i < pScene->mNumMeshes; i++) {
			aiMesh* m = pScene->mMeshes[i];
			Meshes[i] = LoadFromAssimp(pDev, m);
		}

		// *Cbuffer stuff not important*

		g_Mats.Model = dx::DirectX::XMMatrixTranspose(dx::DirectX::XMMatrixTranslation(0, 0, 0) * dx::DirectX::XMMatrixRotationRollPitchYaw(dx::DirectX::XMConvertToRadians(50), dx::DirectX::XMConvertToRadians(20), dx::DirectX::XMConvertToRadians(0)) * dx::DirectX::XMMatrixScaling(1, 1, 1));
		g_Mats.Projection = dx::DirectX::XMMatrixTranspose(dx::DirectX::XMMatrixPerspectiveFovLH(dx::DirectX::XMConvertToRadians(45), fAspect, 0.01f, 100));
		g_Mats.View = dx::DirectX::XMMatrixTranspose(dx::DirectX::XMMatrixLookAtLH({1,5,-1}, {0,0,0}, {0,1,0}));
		/*
			matrix m_Projection; //
			matrix m_View; // shifts universe to camera
			matrix m_Model; // looks
		*/
		dx::D3D11_BUFFER_DESC CBufferDesc{};
		CBufferDesc.BindFlags = dx::D3D11_BIND_CONSTANT_BUFFER;
		CBufferDesc.ByteWidth = sizeof(Mats);
		CBufferDesc.Usage = dx::D3D11_USAGE_DYNAMIC;
		CBufferDesc.CPUAccessFlags = dx::D3D11_CPU_ACCESS_WRITE;
		dx::D3D11_SUBRESOURCE_DATA CBufferData{};
		CBufferData.pSysMem = &g_Mats;
		pDev->CreateBuffer(&CBufferDesc, &CBufferData, &CBuffer);
		pRoot = BuildNode(nullptr, pScene->mRootNode);

	}


	dx::ID3D11Buffer* CBuffer{};
	void SetupCam(dx::ID3D11DeviceContext* Context) {
		Context->VSSetConstantBuffers(0, 1, &CBuffer);
	}
	void Draw(dx::ID3D11DeviceContext* Context) {
		if (!pRoot) return;
		
		auto currentMat = dx::DirectX::XMMatrixIdentity() * dx::DirectX::XMMatrixScaling(1, 1, 1) * dx::DirectX::XMMatrixTranslation(0, -15, 0); // representation of like the "where should I be placed" type operation.
		GoDraw(Context, pRoot, 0, currentMat);
	}

private:
	lagGeometry LoadFromAssimp(dx::ID3D11Device* pDev, aiMesh* m) {
		legit::u64 VSize = m->mNumVertices * 3;
		float* Array = new float[VSize];
		auto iSize = m->mNumFaces * 3; // 3 tris * Num Of Faces
		legit::u32* IArray = new legit::u32[iSize];
		for (int i = 0; i < m->mNumVertices; i++) {
			Array[i * 3 + 0] = m->mVertices[i].x;
			Array[i * 3 + 1] = m->mVertices[i].y;
			Array[i * 3 + 2] = m->mVertices[i].z;
		}
		for (int i = 0; i < m->mNumFaces; i++) {
			IArray[i * 3 + 0] = m->mFaces[i].mIndices[0];
			IArray[i * 3 + 1] = m->mFaces[i].mIndices[1];
			IArray[i * 3 + 2] = m->mFaces[i].mIndices[2];
		}
		auto res = lagGeometry(pDev, Array, VSize * sizeof(float), IArray, iSize, sizeof(float) * 3);
		delete[] IArray;
		delete[] Array;
		return res;
	}
	std::vector<lagGeometry> Meshes{};
	struct strNode {
		std::vector<int> ContrlMshs{};
		std::string Name;
		dx::DirectX::XMMATRIX NodeMat{dx::DirectX::XMMatrixIdentity()};
		strNode* pParent;
		std::vector<strNode*> pChildren;
		~strNode() {
			for (auto& child : pChildren) {
				delete child;
			}
			pChildren.clear();
		}
	};
	void GoDraw(dx::ID3D11DeviceContext* Context, strNode* pParent, int depth, dx::DirectX::XMMATRIX Accrewed) {
		if (pParent->Name == "Cube") return;
		Accrewed = Accrewed * pParent->NodeMat;
/*		for (int i = 0; i < depth; i++) {
			netLogger::Send({"\t"});
		}*/
		netLogger::Send({"%s: %s\n", pParent->Name.c_str(), MatrixToString(Accrewed).c_str()});
		dx::D3D11_MAPPED_SUBRESOURCE Sub{};
		Context->Map(CBuffer, 0, dx::D3D11_MAP_WRITE_DISCARD, 0, &Sub);
		Mats m{g_Mats};
		m.Model = dx::DirectX::XMMatrixTranspose(Accrewed); // a lil not efficient but I mean come on we are rendering one fucking model lmao.
		memcpy(Sub.pData, &m, sizeof(Mats));
		Context->Unmap(CBuffer, 0);
		Context->VSSetConstantBuffers(0, 1, &CBuffer);
		for (const auto& r : pParent->ContrlMshs) {
			Meshes[r].Draw(Context);
		}
		for (const auto& c : pParent->pChildren) {
			GoDraw(Context, c, depth + 1, Accrewed); // yikes lmao.
		}
	}
	std::string MatrixToString(dx::DirectX::XMMATRIX matrix) {
	// Stored in row-major format, you access elements like this:
	// M11 M12 M13 M14
	// M21 M22 M23 M24
	// etc.

	// Use an XMFLOAT4X4 helper structure for easier element access
		dx::DirectX::XMFLOAT4X4 float4x4;
		dx::DirectX::XMStoreFloat4x4(&float4x4, matrix);

		std::stringstream ss;
		ss << "Matrix:\n";
		ss << float4x4.m[0][0] << ", " << float4x4.m[0][1] << ", " << float4x4.m[0][2] << ", " << float4x4.m[0][3] << "\n";
		ss << float4x4.m[1][0] << ", " << float4x4.m[1][1] << ", " << float4x4.m[1][2] << ", " << float4x4.m[1][3] << "\n";
		ss << float4x4.m[2][0] << ", " << float4x4.m[2][1] << ", " << float4x4.m[2][2] << ", " << float4x4.m[2][3] << "\n";
		ss << float4x4.m[3][0] << ", " << float4x4.m[3][1] << ", " << float4x4.m[3][2] << ", " << float4x4.m[3][3];

		return ss.str();
	}
	static dx::DirectX::XMMATRIX AiToDx(const aiMatrix4x4& m) {
		return dx::DirectX::XMMATRIX(
			m.a1, m.b1, m.c1, m.d1,
			m.a2, m.b2, m.c2, m.d2,
			m.a3, m.b3, m.c3, m.d3,
			m.a4, m.b4, m.c4, m.d4
		);
	}
	strNode* pRoot = nullptr;
	strNode* BuildNode(strNode* pParent, aiNode* pNode) {
		strNode* newNode = new strNode();
		newNode->Name = std::string(pNode->mName.C_Str());
		newNode->pParent = pParent;
		newNode->NodeMat = AiToDx(pNode->mTransformation);
		for (int i = 0; i < pNode->mNumMeshes; i++) {
			newNode->ContrlMshs.push_back(pNode->mMeshes[i]);
		}
		if (pNode->mNumChildren == 0) {
			return newNode;
		}
		for (int i = 0; i < pNode->mNumChildren; i++) {
			newNode->pChildren.push_back(BuildNode(newNode, pNode->mChildren[i]));
		}
		return newNode;
	}
};
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
/*		auto Model = dx::DirectX::XMMatrixTranspose(dx::DirectX::XMMatrixTranslation(0, 0, 0) * dx::DirectX::XMMatrixRotationRollPitchYaw(dx::DirectX::XMConvertToRadians(50), dx::DirectX::XMConvertToRadians(20), dx::DirectX::XMConvertToRadians(0)) * dx::DirectX::XMMatrixScaling(1, 1, 1));
		auto Projection = dx::DirectX::XMMatrixTranspose(dx::DirectX::XMMatrixPerspectiveFovLH(dx::DirectX::XMConvertToRadians(90.f), 16.0 / 9.0, 0.01f, 100));
		auto View = dx::DirectX::XMMatrixTranspose(dx::DirectX::XMMatrixLookAtLH({2,2,-2}, {0,0,0}, {0,1,0}));
		/*
			matrix m_Projection; //
			matrix m_View; // shifts universe to camera
			matrix m_Model; // looks
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
		Context->VSSetConstantBuffers(0, 1, &CBuffer)
		;*/
		int i = 0;
		dx::D3D11_RASTERIZER_DESC RDesc{};
		RDesc.CullMode = dx::D3D11_CULL_NONE;
		RDesc.FillMode = dx::D3D11_FILL_SOLID;
		RDesc.MultisampleEnable = false;
		RDesc.AntialiasedLineEnable = false;
		dx::ID3D11RasterizerState* pState{};
		Device->CreateRasterizerState(&RDesc, &pState);
		Context->RSSetState(pState);
/*		
		lagGeometry g{};
		CreateQuadVerts(Device, g);
*/
		
		lagModel m{Device, (float)width / (float)height};
/*		dx::D3D11_TEXTURE2D_DESC BBDESC{};
		BackBufferRaw->GetDesc(&BBDESC);

		dx::ID3D11Texture2D* pDepthStencil = NULL;
		dx::D3D11_TEXTURE2D_DESC descDepth;
		descDepth.Width = BBDESC.Width;
		descDepth.Height = BBDESC.Height;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = dx::DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = dx::D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = dx::D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		hr = Device->CreateTexture2D(&descDepth, NULL, &pDepthStencil);
		dx::D3D11_DEPTH_STENCIL_DESC dsDesc;

// Depth test parameters
		dsDesc.DepthEnable = true;
		dsDesc.DepthWriteMask = dx::D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = dx::D3D11_COMPARISON_LESS;

		// Stencil test parameters
		dsDesc.StencilEnable = false;
		dsDesc.StencilReadMask = 0xFF;
		dsDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing
		dsDesc.FrontFace.StencilFailOp = dx::D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilDepthFailOp = dx::D3D11_STENCIL_OP_INCR;
		dsDesc.FrontFace.StencilPassOp = dx::D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilFunc = dx::D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing
		dsDesc.BackFace.StencilFailOp = dx::D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilDepthFailOp = dx::D3D11_STENCIL_OP_DECR;
		dsDesc.BackFace.StencilPassOp = dx::D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilFunc = dx::D3D11_COMPARISON_ALWAYS;
		// Create depth stencil state
		dx::ID3D11DepthStencilState* pDSState;
		Device->CreateDepthStencilState(&dsDesc, &pDSState);
		Context->OMSetDepthStencilState(pDSState, 1);
		dx::D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		descDSV.Format = dx::DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		descDSV.ViewDimension = dx::D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;

		// Create the depth stencil view
		dx::ID3D11DepthStencilView* pDSV;
		hr = Device->CreateDepthStencilView(pDepthStencil, // Depth stencil texture
			&descDSV, // Depth stencil desc
			&pDSV);  // [out] Depth stencil view

// Bind the depth stencil view
		Context->OMSetRenderTargets(1,          // One rendertarget view
			&BackBufferRTV,      // Render target view, created earlier
			pDSV);     // Depth stencil view for the render target*/
		while (!ShouldWindowClose) {
			WindowDef(ShouldWindowClose);
			float fColor[4] = {0,0,0,1};
			Context->ClearRenderTargetView(BackBufferRTV, fColor);
			//Context->ClearDepthStencilView(pDSV, dx::D3D11_CLEAR_DEPTH, 1,0);
			//g.Draw(Context);
			m.Draw(Context);
			SwapChain->Present(0, 0);
		}
	}
	netLogger::Shutdown();
	netPlatConfig::ShutdownClass();
}