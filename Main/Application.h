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

/*
* Scene Graphs are not as complicated as you make them out to be. Every specific entity (CMyCube) has to have an attached entity to it. This is its children and thus effected by ITS Translations.
* So as an example this could be a definition of CMyCube:
* class CMyCube{
* ... Rendering Tech
* private:
*	std::vector<CMyCube*> m_Children;
* public:
*	std::vector<CMyCube*> GetChildren() { return m_Children; }
* };
* Then what is necessary is modifying attached objects in relevence to the base object. So for example a tyre on a vehicle might have a definition in which its a Child of the Cube.
* But When the Cube/Vehicle is created and the Cube moves. the Child has to update its positional vector as well. (if this makes sense!) So whereas a cube's definition might define {0,0,0} and a tyre's offset would be {1, 0, 1} (as an example of FRONT_LEFT_TYRE). This means that in global space it gets transfered and moved around with this included in mind so that its ALWAYS moving with positional changes. 
* I thought it was more complex but looking at Unreal and Unity it just shows me that when you make an Entity it has children. The children must exist somewhere else it wouldn't work with the Scene Graph System.
* Very confusing but actually kinda also not confusing. probably next though will be a vehicle MAYBE! Or something that enforces multiple entities in relevence to another. (we can also see this visually in Blender!)
*/

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
#include "imgui.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
class CMainWindow{
public:
	CMainWindow(CWinArgs* args) : m_Window(args, L"lagWindow", L"GameWin32.exe", WindowProc) {
		m_pWindow = this;
		ShowWindow(m_Window.GetWindowHandle(), m_Window.GetWindowArgs()->GetCmdShow());
	}
	bool g_Close = false;
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
			return true;
		}
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
#ifndef SHADER_MAIN
	#define SHADER_MAIN "lagMain"
#endif
class lagCompilerDX11 {
public:
	lagShaderBytecode Compile(const wchar_t* path, const char* entryPoint, const char* ShaderVersion, UINT flags) {
		ID3DBlob* ErrorBlob{},* Blob{};
		HRESULT hr;
		D3D_SHADER_MACRO macro{};
		macro.Name = "LAG";
		macro.Definition = SHADER_MAIN;
		D3D_SHADER_MACRO Macros[] = {
			macro,
			{NULL, NULL}
		};
		hr = D3DCompileFromFile(path, Macros, NULL, entryPoint, ShaderVersion, flags, 0, &Blob, &ErrorBlob); // This might be just static behavior. 
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
using ContextSignature = ID3D11DeviceContext* (*)();
static DeviceSignature GetDevice;
static ContextSignature GetContext;
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
enum class eValueShaderProgram { // These are the generics.
	VERTEX,
	FRAGMENT,
	COMPUTE,
	MAX
};
template<size_t N>
struct FixedString {
	char buf[N];
	constexpr FixedString(const char(&str)[N]) {
		for (size_t i = 0; i < N; ++i) buf[i] = str[i];
	}
};
template<eValueShaderProgram prog> struct ShaderOfType {
private:
	DBG_ONLY(static constexpr const char* PRV[] = { "VERTEX","FRAGMENT","COMPUTE" };)
public:
	static constexpr eValueShaderProgram Val = prog;
	DBG_ONLY(static constexpr const char* DebugName = PRV[(int)prog];)
};
template<typename T> struct ShaderType : public ShaderOfType<eValueShaderProgram::COMPUTE> {};
template<typename T> struct ShaderType<T*> : public ShaderType<T>{

};
template<> struct ShaderType<lagVertexShader> : public ShaderOfType<eValueShaderProgram::VERTEX> {};
template<> struct ShaderType<lagFragmentShader> : public ShaderOfType<eValueShaderProgram::FRAGMENT>{};
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
	DirectX::XMFLOAT2 m_Color;
};
enum class eVertexType {
	VECTOR2,
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
	static constexpr int SizeOfType = sizeof (Type);
	static constexpr DXGI_FORMAT m_TypeFormat = F;
};
template<typename T, DXGI_FORMAT FORMAT>
struct BaseEvaluator {
	using Type = T;
	static constexpr VertexOfType<Type, FORMAT> VertexInformation;
};
template<eVertexType T> struct VertexEvaluater : public BaseEvaluator<void*, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN> { static_assert(true && "Type is unknown, Unsafely casting to void ptr!"); };
template<> struct VertexEvaluater<eVertexType::VECTOR2> : public BaseEvaluator<DirectX::XMFLOAT2, DXGI_FORMAT_R32G32_FLOAT>{ };
template<> struct VertexEvaluater<eVertexType::VECTOR3> : public BaseEvaluator<DirectX::XMFLOAT3, DXGI_FORMAT_R32G32B32_FLOAT>{ };
template<> struct VertexEvaluater<eVertexType::VECTOR4> : public BaseEvaluator<DirectX::XMFLOAT4, DXGI_FORMAT_R32G32B32A32_FLOAT>{ };

enum class eStaticVertexNames {
	POSITION,
	COLOR,
	UVCOORD,
	SIZE_OF
};
inline constexpr const char* VERTEX_NAMES[] = {
	"POSITION",
	"COLOR",
	"UVCOORD",
};
static_assert((int)eStaticVertexNames::SIZE_OF == sizeof(VERTEX_NAMES)/sizeof(VERTEX_NAMES[0]) && "Not enough VERTEX_NAME formats");
template<eStaticVertexNames PRIM> struct VrtxToStr {
	static constexpr const char* GetName() {
		return VERTEX_NAMES[(int)PRIM];
	}
};
template<eStaticVertexNames NAME, eVertexType T, int Index>
struct lagStaticVertexDeclaration {
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
template<typename... V> struct lagStaticVertexFormat {
	static lagInputAssembler* CreateLayout(const lagShaderBytecode& byteCode) {
		constexpr std::array<D3D11_INPUT_ELEMENT_DESC, sizeof...(V)> arr = {
			VertexFormatConversion<V>::GetDescription()...
		};
		lagInputAssembler* m_InputLayout = new lagInputAssembler(arr.data(), arr.size(), byteCode);
		return m_InputLayout;
	}
};
template<eVertexType... Types> struct lagVertex {
public:
	template<eVertexType... Types>
	lagVertex() {
		auto size = (VertexEvaluater<Types>::Type)...
	}
private:
	void* m_Buffer;
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
struct Constant {
	DirectX::XMMATRIX m_Projection;
	DirectX::XMMATRIX m_View;
	DirectX::XMMATRIX m_Model;
};
class lagConstantBuffer {
public:
	lagConstantBuffer() {
		D3D11_BUFFER_DESC Desc{};
		Desc.Usage = D3D11_USAGE_DYNAMIC; // DYNAMIC BUFFER
		Desc.ByteWidth = sizeof(Constant);
		Desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		Desc.StructureByteStride = 0;
		Desc.MiscFlags = 0; // We do not have misc flags. 
		GetDevice()->CreateBuffer(&Desc, NULL, &m_Buffer); 
	}
	Constant* Map(D3D11_MAP MapType = D3D11_MAP_WRITE_DISCARD, UINT MapFlags = 0) {
		D3D11_MAPPED_SUBRESOURCE Res{};
		GetContext()->Map(m_Buffer, 0, MapType, MapFlags, &Res);
		return (Constant*)Res.pData;
	}
	void Unmap() {
		GetContext()->Unmap(m_Buffer, 0);
	}
	ID3D11Buffer* GetBuffer() {
		return m_Buffer;
	}
	ID3D11Buffer** GetBufferPtr() {
		return &m_Buffer;
	}
	~lagConstantBuffer() {
		if (m_Buffer) m_Buffer->Release();
	}
private:
	ID3D11Buffer* m_Buffer = nullptr;
};

// Synopsis from Chili's Video. Scene graphs are only for per-object translations. Meaning that Scene Graphs != Meshes individually. 
// Scene graphs only represent like for example, how a certain model moves. 

template<typename T> class vertBuffer : public lagVertexBuffer {
public:
	vertBuffer(std::vector<T> Vertices) : lagVertexBuffer(Vertices.data(), Vertices.size(), sizeof(T)) {}
private:

};
class CMesh {
public:
	CMesh(std::vector<Vertex> Vertices, std::vector<unsigned int> Indices) : m_Vertices(Vertices), m_Indices(Indices.data(), Indices.size()){
		std::cout << "Using CMESH\n\0";
	}
	void Draw() {
		UINT pStrides = sizeof(Vertex);
		UINT pOffset = 0;
		GetContext()->IASetVertexBuffers(0, 1, m_Vertices.GetBufferPtr(), &pStrides, &pOffset);
		GetContext()->IASetIndexBuffer(m_Indices.GetBuffer(), DXGI_FORMAT_R32_UINT, 0); // I would assume thats the type lmao.
		GetContext()->DrawIndexed(m_Indices.GetSize(), 0, 0);
	}
	~CMesh() {

	}
private:
	vertBuffer<Vertex> m_Vertices;
	lagIndexBuffer m_Indices;
};
class CModel{
public:

private:
	std::vector<CMesh> m_Meshes;
};
#include "stb_image.h"
class CSTBIImage {
private:
	static void Copy(CSTBIImage& self, const CSTBIImage& source) {
		self.Channels = source.Channels;
		self.Width = source.Width;
		self.Height = source.Height;
		size_t Size = static_cast<size_t>(self.Channels) * self.Width * self.Height;
		self.m_Mem = new stbi_uc[Size];
		memcpy(self.m_Mem, source.m_Mem, Size);
	}
	static void Move(CSTBIImage& self, CSTBIImage&& rhs) {
		self.Channels = rhs.Channels;
		self.Height = rhs.Height;
		self.Width = rhs.Width;
		self.m_Mem = rhs.m_Mem;
		rhs.m_Mem = nullptr;
	}
public:
	CSTBIImage(const char* path, int TargetedChannels) {
		this->m_Mem = stbi_load(path, &this->Width, &this->Height, &Channels, TargetedChannels);
	}
	CSTBIImage(const CSTBIImage& image) noexcept : m_Mem(nullptr) {
		Copy(*this, image);
	}
	CSTBIImage& operator=(const CSTBIImage&& image) noexcept {
		Copy(*this, image);
		return *this;
	}
	CSTBIImage(CSTBIImage&& rhs) noexcept {
		Move(*this, std::forward<CSTBIImage>(rhs));
	}
	CSTBIImage& operator=(CSTBIImage&& rhs) noexcept {
		Move(*this, std::forward<CSTBIImage>(rhs));
		return *this;
	}
	int GetWidth() const {
		return this->Width;
	}
	int GetHeight() const {
		return this->Height;
	}
	int GetChannels() const {
		return this->Channels;
	}
	stbi_uc* GetImage() {
		return this->m_Mem;
	}
	~CSTBIImage() {
		if (m_Mem) {
			stbi_image_free(m_Mem);
		}
	}
private:
	int Width{}, Height{}, Channels{};
	stbi_uc* m_Mem;
};
#include "thirdparty/imgui-dock/imgui.h"
#include "thirdparty/imgui-dock/backends/imgui_impl_win32.h"
#include "thirdparty/imgui-dock/backends/imgui_impl_dx11.h"

class CDebug {
public:
	CDebug() {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_DockingEnable;
		ImGui_ImplWin32_Init(CMainWindow::Get().GetRawWindow()->GetWindowHandle());
		ImGui_ImplDX11_Init(GetDevice(), GetContext());
	}
	void BeginFrame() {
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

	}
	void EndFrame() {
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
	~CDebug() {
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
private:

};

class CMyCube {
public:
	CMyCube() {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile("C:\\Users\\codyc\\OneDrive\\Docs from Gaming PC\\Documents\\TestModels\\cube.obj", aiPostProcessSteps::aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenUVCoords);
		auto CPUVb = MakeModel(scene);
		auto cpuIb = MakeIndices(scene);
		this->m_Mesh = new CMesh(CPUVb, cpuIb);
		m_LocalMtx = DirectX::XMMatrixTranslation(0, 1, 2) * DirectX::XMMatrixRotationY(50);
		InitVertexInformation();
		m_Buffer = new lagConstantBuffer();
		InitTexture();
	}
	void InitVertexInformation() {
		lagShaderBytecode VS = lagShaderCompiler::Compile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\vs.hlsl", SHADER_MAIN, "vs_5_0", D3DCOMPILE_DEBUG);
		m_VertexShader = new lagVertexShader(VS);
		lagShaderBytecode PS = lagShaderCompiler::Compile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\ps.hlsl", SHADER_MAIN, "ps_5_0", D3DCOMPILE_DEBUG);
		m_FragShader = new lagFragmentShader(PS);
#if LE_DEBUG
		std::cout << "Shaders of Types : " << ShaderType<decltype(m_VertexShader)>::DebugName << " and " << ShaderType<decltype(m_FragShader)>::DebugName << " were created. ";
#endif
		m_InputAssembler = Format::CreateLayout(VS); // This might be weird. 
	}
	void InitTexture() {
		const char* path = "D:\\xenia_canary\\Games\\Midnight Club - Los Angeles - Complete Edition (USA, Europe)\\rockstar_logo_64x64.png";
		CSTBIImage load = CSTBIImage(path, 4);
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = load.GetWidth();
		textureDesc.Height = load.GetHeight();
		textureDesc.MipLevels = 1; // im not gonna abstract these yet.
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		D3D11_SUBRESOURCE_DATA textureResourceData{};
		textureResourceData.pSysMem = load.GetImage();
		textureResourceData.SysMemPitch = load.GetWidth() * sizeof(UINT);
		HRESULT hr = GetDevice()->CreateTexture2D(&textureDesc, &textureResourceData, &m_TextureBuffer);
		if (hr != S_OK) {
			std::cout  << "Failed: [CreateTexture2D] HR: " <<  GetLastError();
			return;
		}
		hr = GetDevice()->CreateShaderResourceView(this->m_TextureBuffer, nullptr, &this->m_ShaderResourceView);
		if (hr != S_OK) {
			this->m_TextureBuffer->Release();
			std::cout << "Failed: [CreateTexture2D] HR: " << GetLastError();
			return;
		}
		D3D11_SAMPLER_DESC SampleDesc{};
		SampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		SampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		SampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		SampleDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		SampleDesc.MipLODBias = 0.0f;
		SampleDesc.MaxAnisotropy = 1;
		SampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		SampleDesc.BorderColor[0] = SampleDesc.BorderColor[1] = SampleDesc.BorderColor[2] = SampleDesc.BorderColor[3] = 0;
		SampleDesc.MinLOD = 0;
		SampleDesc.MaxLOD = D3D11_FLOAT32_MAX;
		hr = GetDevice()->CreateSamplerState(&SampleDesc, &this->m_SamplerState);
		if (hr != S_OK) {
			this->m_TextureBuffer->Release();
			std::cout << "Failed: [CreateSamplerState] HR: " << GetLastError();
			return;
		}
	}
	DirectX::XMMATRIX& GetMtx() {
		return m_LocalMtx;
	}
	lagConstantBuffer* GetBuffer() {
		return this->m_Buffer;
	}
private:
	DirectX::XMFLOAT3 POSITION = {0,0,0};
	DirectX::XMFLOAT3 ROTATION = { 0,0,0 };
	DirectX::XMFLOAT3 SCALE = { 1,1,1 };
	void ImGuiXmFloat(DirectX::XMFLOAT3 f) {
		float x[3] = { f.x, f.y, f.z };
	}
public:
	void Draw(ID3D11DeviceContext* pContext) {
		const UINT stride = sizeof Vertex;
		const UINT offset = 0;
		ImGui::Begin("Object");
		float pos[3] = { POSITION.x, POSITION.y, POSITION.z };
		float rot[3] = { ROTATION.x, ROTATION.y, ROTATION.z };
		float scale[3] = { SCALE.x, SCALE.y, SCALE.z };
		if (ImGui::InputFloat3("Position", pos)) {
			POSITION.x = pos[0];
			POSITION.y = pos[1];
			POSITION.z = pos[2];
		}
		if (ImGui::InputFloat3("Rotation", rot)) {
			ROTATION.x = rot[0];
			ROTATION.y = rot[1];
			ROTATION.z = rot[2];
		}
		if (ImGui::InputFloat3("Scale", scale)) {
			SCALE.x = scale[0];
			SCALE.y = scale[1];
			SCALE.z = scale[2];
		}
		ImGui::End();

		m_LocalMtx = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&POSITION)) * DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&ROTATION)) * DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&SCALE));

		pContext->IASetInputLayout(m_InputAssembler->GetLayout());
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // I would assume.

		pContext->PSSetShader(this->m_FragShader->GetShader(), nullptr, 0); // Signature Similar?
		pContext->PSSetShaderResources(0, 1, &m_ShaderResourceView);
		pContext->PSSetSamplers(0, 1, &m_SamplerState);


		pContext->VSSetShader(this->m_VertexShader->GetShader(), nullptr, 0);
		pContext->VSSetConstantBuffers(0, 1, m_Buffer->GetBufferPtr());
		
		m_Mesh->Draw();
	}
	~CMyCube() {
		delete m_VertexShader;
		delete m_FragShader;
		delete m_InputAssembler;
		delete m_IB;
		delete m_VB;
		delete m_Mesh;
	}
	DirectX::XMMATRIX& GetLocal() {
		return this->m_LocalMtx;
	}
private: // Texture Specifications;
	ID3D11Texture2D* m_TextureBuffer;
	ID3D11ShaderResourceView* m_ShaderResourceView;
	ID3D11SamplerState* m_SamplerState;
private:
	DirectX::XMMATRIX m_LocalMtx = DirectX::XMMatrixIdentity();
	std::vector<Vertex> MakeModel(const aiScene* scene) {
		if (scene->HasMeshes()) {
			aiMesh* mesh = scene->mMeshes[0]; // Of course this is a static check because I already know how many models I have. Ideally I'd check this. 
			std::vector<Vertex> Mesh;
			for (int i = 0; i < mesh->mNumVertices; i++) {
				aiVector3D v3D = mesh->mVertices[i];
				Vertex v{};
				v.m_Positions = { v3D.x, v3D.y, v3D.z };
				v.m_Color = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
				Mesh.push_back(v);
			}
			return Mesh;
		}
		std::cout << "Scene does not have a mesh!\n\0";
		return { };
	}
	std::vector<unsigned int> MakeIndices(const aiScene* scene) {
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
	// This can be per-object? 
	lagVertexShader* m_VertexShader = nullptr;
	lagFragmentShader* m_FragShader = nullptr;
	//Mesh
	using Format = lagStaticVertexFormat<
		lagStaticVertexDeclaration<eStaticVertexNames::POSITION, eVertexType::VECTOR3, 0>,
		lagStaticVertexDeclaration<eStaticVertexNames::UVCOORD, eVertexType::VECTOR2, 0>
	>;
	CMesh* m_Mesh;
	lagInputAssembler* m_InputAssembler = nullptr;
	lagVertexBuffer* m_VB = nullptr;
	lagIndexBuffer* m_IB = nullptr;

	// This is a constant buffer which either is specified by the object or its specified by the shader. The Shader USES the buffer but it doesn't know what it actually would be. 
	lagConstantBuffer* m_Buffer = nullptr;
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
	DirectX::XMFLOAT3 m_Position = {2,2,10}; // Target and Position can never meet. 
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
	static ID3D11DeviceContext* __GetContext() {
		return Get().m_pContext;
	}
	void InitDeviceLayer() {
		GetDevice = __GetDevice;
		GetContext = __GetContext;
		lagShaderCompiler::InitClass();
	}

	void SetupConstantBuffer() {
		//D3D11_BUFFER_DESC Desc{};
		//Desc.Usage = D3D11_USAGE_DYNAMIC; // DYNAMIC BUFFER
		//Desc.ByteWidth = sizeof(Constant);
		//Desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
		//Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		//Desc.StructureByteStride = 0;
		//Desc.MiscFlags = 0; // We do not have misc flags. 
		//HRESULT hr;
		//hr = m_pDevice->CreateBuffer(&Desc, nullptr, &m_ConstantBuffer);
		//if (FAILED(hr)) {
		//	std::cout << "Buffer could not make constant buffer. " << hr << "\n\0";
		//	//CApplication::Get().GetWindow()->g_Close = true;
		//	return;
		//}
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

		ID3D11Texture2D* m_Backbuffer = nullptr;

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
		/* 
		* Slowly getting to textures is scary but I think it'll be fun once I can understand how they work and allat. 
		*/
		D3D11_TEXTURE2D_DESC depthBufferDesc{};
		m_Backbuffer->GetDesc(&depthBufferDesc);
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		ID3D11Texture2D* depthBuffer{};
		m_pDevice->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);
		DepthView = {};
		hr = m_pDevice->CreateDepthStencilView(depthBuffer, nullptr, &DepthView);

		D3D11_DEPTH_STENCIL_DESC depthstencildesc = {};
		depthstencildesc.DepthEnable = TRUE;
		depthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthstencildesc.DepthFunc = D3D11_COMPARISON_LESS;


		m_pDevice->CreateDepthStencilState(&depthstencildesc, &DepthStencilState);
		if (FAILED(hr)) {
			std::cout << "[BACKBUFFER_ACCESS]: ID3D11DepthStencilView failed to Create!" << hr << "\n\0";
			__debugbreak();
			return;
		}
		m_pContext->OMSetRenderTargets(1, &m_View, DepthView); // NO DEPTH.
		m_Backbuffer->Release();
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
	ID3D11DepthStencilView* DepthView{};
	ID3D11DepthStencilState* DepthStencilState{};

	void CompileShaders() {

	}
	void Clear() {
		const float m_fClear[4] = { 0.145,0.145,0.16,1.0f };
		m_pContext->ClearRenderTargetView(m_View, m_fClear);

		m_pContext->ClearDepthStencilView(DepthView, D3D11_CLEAR_DEPTH, 1.0, 0);
		m_pContext->OMSetDepthStencilState(DepthStencilState, 0);
	}
	void Update() {

		UpdateCameraPosition();

		
		
		Constant* pConst = m_MyCube->GetBuffer()->Map();

		Constant constant{};

		constant.m_Model = m_MyCube->GetMtx(); // This is based on the Object.
		constant.m_View = m_Camera->WhatWeLookingAt(); // This is based on the Camera
		float fAspect = viewport.Width / viewport.Height; 
		constant.m_Projection = m_Camera->GetPerspectiveMtx(fAspect); // This is based on the Camera && Display.

		constant.m_Model = XMMatrixTranspose(constant.m_Model);
		constant.m_View = XMMatrixTranspose(constant.m_View);
		constant.m_Projection = XMMatrixTranspose(constant.m_Projection);

		memcpy(pConst, &constant, sizeof(Constant));

		m_MyCube->GetBuffer()->Unmap();

		m_MyCube->Draw(m_pContext);

	}
	void Present() {
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
	camCamera* m_Camera = nullptr; // this is not entirely relevent to the renderer. 


	CMyCube* m_MyCube = nullptr;

	CMyCube* m_Cube2 = nullptr;

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
		m_Debug = new CDebug();
#else
		printf("[Error]: Unknown/Unanticipated Platform. We are unable to support your operating system or platform.");
		this->m_bStopRunning = true;
#endif // LE_WIN
		return;
	}
	void Update() {
		CRenderer::Get().Clear();
		m_Debug->BeginFrame();

		this->m_bStopRunning = m_Window->g_Close;
		m_Window->Poll();

		CRenderer::Get().Update();
		m_Debug->EndFrame();
		CRenderer::Get().Present();
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
		delete m_Debug;
		delete m_Window;
		CLogger::Shutdown();
	}
	CMainWindow* GetWindow() { return m_Window; }
private:
	CDebug* m_Debug = nullptr;
	CMainWindow* m_Window = nullptr;
#ifdef LE_WIN32
	CWinArgs*
#else 
	legit::fwCmdArgs*
#endif
	m_Arguments = nullptr;
	bool m_bStopRunning = false;
};