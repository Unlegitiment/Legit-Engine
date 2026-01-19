#pragma once
#ifdef USE_RENDERER
#define NOMINMAX
#include <LECore/core/args.h>
#include "WindowHandling/CWindow.h"
#include <iostream>
#include <LECore/maths/vec2.h>
#include <LECore/le_types.h>
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
#include "imgui.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include <d3d9.h>
#include <d3d11.h>
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
			std::cout << "[lagCompilerDX11] " << (char*)ErrorBlob->GetBufferPointer() << std::endl;
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
template<eValueShaderProgram prog> struct ShaderOfType {
private:
	DBG_ONLY(static constexpr const char* PRV[] = { "VERTEX","FRAGMENT","COMPUTE" };)
public:
	static constexpr eValueShaderProgram Val = prog;
	DBG_ONLY(static constexpr const char* DebugName = PRV[(int)prog];)
};
template<typename T> struct ShaderType : public ShaderOfType<eValueShaderProgram::COMPUTE> {};
template<typename T> struct ShaderType<T*> : public ShaderType<T>{ };
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
	static constexpr size_t SizeOfType = sizeof (Type);
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
	NORMALS,
	SIZE_OF
};
inline constexpr const char* VERTEX_NAMES[] = {
	"POSITION",
	"COLOR",
	"UVCOORD",
	"NORMALS",
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
	static constexpr const size_t Size() {
		size_t s{0};
		s += ((sizeof(VertexEvaluater<V::Type>::Type)), ...);
		// HACK! Vertices that don't comply are multiplied by the amount in order to meet the threshold for the assert. 12 * 4 = 48, assert pass. 
		s *= sizeof...(V);
		return s;
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
using lagRenderSurface = void*; // could also define this as a class. And just make it convertable from CWindow(not inside of the class lmao goofy)
class lagDevice final{
public:
	using DebugFunction = void(*)(HRESULT, const char*);
	static void Init(lagRenderSurface RenderSurface) {
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
		Desc.OutputWindow = (HWND)RenderSurface;
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
			&m_pSwapChain, &m_pDevice, &ReturnLevel, &m_pContext
		);
		if (hr != S_OK) {
			std::cout << "Failed to init D3D11, " << hr << std::endl;
		}
	}
	static void __df(HRESULT hr, const char* func) {
		printf("Operation Failed: %s, with result: %x", func, hr);
	}
	static inline DebugFunction m_DbgFunction{__df};
#define NotifyIfFailed(hr, retVal) if(FAILED(hr)) { if(m_DbgFunction) {m_DbgFunction(hr, __FUNCTION__);} __debugbreak(); return retVal; } 
	static ID3D11Texture2D* GetBackBuffer() {
		ID3D11Texture2D* m_Backbuffer = nullptr;
		HRESULT hr;
		hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_Backbuffer));
		NotifyIfFailed(hr, nullptr);
		return m_Backbuffer;
	}
	static void SetViewports(const std::vector<D3D11_VIEWPORT>& Viewports) {
		m_pContext->RSSetViewports(Viewports.size(), Viewports.data());
	}
	static ID3D11SamplerState* CreateSamplerState(const D3D11_SAMPLER_DESC* pDesc) {
		ID3D11SamplerState* pState{nullptr};
		HRESULT hr = m_pDevice->CreateSamplerState(pDesc, &pState);
		NotifyIfFailed(hr, nullptr);
		return pState;
	}
	static void SetFragmentShaderResources(UINT StartSlot, const std::vector<ID3D11ShaderResourceView*>& Resources) {
		m_pContext->PSSetShaderResources(StartSlot, Resources.size(), Resources.data());
	}
	static void SetFragmentSamplers(UINT StartSlot, const std::vector<ID3D11SamplerState*> States) {
		m_pContext->PSSetSamplers(StartSlot, States.size(), States.data());
	}
	static ID3D11ShaderResourceView* CreateShaderResourceView(ID3D11Resource* pResource, D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc = nullptr) {
		HRESULT hr{S_OK};
		ID3D11ShaderResourceView* pView{};
		hr = m_pDevice->CreateShaderResourceView(pResource, pDesc, &pView);
		NotifyIfFailed(hr, nullptr);
		return pView;
	}
	static ID3D11DepthStencilState* CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDesc) {
		ID3D11DepthStencilState* pState{nullptr};
		HRESULT hr = m_pDevice->CreateDepthStencilState(pDesc, &pState);
		NotifyIfFailed(hr, nullptr);
		return pState;
	}
	static ID3D11Texture2D* CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc, D3D11_SUBRESOURCE_DATA* pData) {
		ID3D11Texture2D* Texture{nullptr};
		HRESULT hr = m_pDevice->CreateTexture2D(pDesc, pData, &Texture);
		NotifyIfFailed(hr, nullptr);
		return Texture;
	}
	static ID3D11DepthStencilView* CreateDepthStencilView(ID3D11Resource* pRes, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc) {
		ID3D11DepthStencilView* pView{nullptr};
		HRESULT hr = m_pDevice->CreateDepthStencilView(pRes, pDesc, &pView);
		NotifyIfFailed(hr, nullptr);
		return pView;
	}
	static void SetRenderTargets(const std::vector<ID3D11RenderTargetView*>& Views, ID3D11DepthStencilView* Depth) {
		m_pContext->OMSetRenderTargets(Views.size(), Views.data(), Depth);
	}
	static void ClearDepthStencilView(ID3D11DepthStencilView* DepthView, UINT ClearFlags = D3D11_CLEAR_DEPTH, float Depth = 1.f, UINT Stencil = 0) {
		m_pContext->ClearDepthStencilView(DepthView, ClearFlags, Depth, Stencil);
	}
	static void ClearRenderTarget(ID3D11RenderTargetView* pView, float r, float g, float b, float a) {
		const float fClear[4] = {r,g,b,a};
		m_pContext->ClearRenderTargetView(pView, fClear);
	}
	static void ClearRenderTargets(const std::vector<ID3D11RenderTargetView*>& Views, float r, float g, float b, float a) {
		const float fClear[4] = {r,g,b,a};
		for (const auto& rtv : Views) {
			m_pContext->ClearRenderTargetView(rtv, fClear);
		}
	}
	static ID3D11RenderTargetView* CreateRenderTarget(ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc) {
		ID3D11RenderTargetView* pRet{};
		HRESULT hr = m_pDevice->CreateRenderTargetView(pResource, pDesc, &pRet);
		NotifyIfFailed(hr, nullptr);
		return pRet;
	}
	static void Present(unsigned long SyncInterval, unsigned long Flags) { // Convert these to standard types!
		HRESULT hr = m_pSwapChain->Present(SyncInterval, Flags);
		NotifyIfFailed(hr, );
	}
	static ID3D11Buffer* CreateBuffer() {

	}
	static void SetVertexShader(ID3D11VertexShader* Shader, ID3D11ClassInstance*const* Instances = nullptr, UINT SizeInstance = 0){
		m_pContext->VSSetShader(Shader, Instances, SizeInstance); 
	}
	static void SetVertexConstantBuffers(UINT StartSlot, const std::vector<ID3D11Buffer*>& Buffers) {
		m_pContext->VSSetConstantBuffers(StartSlot, Buffers.size(), Buffers.data());
	}
	static void SetFragmentShader(ID3D11PixelShader* Shader, ID3D11ClassInstance* const* Instances = nullptr, UINT SizeInstance = 0) {
		m_pContext->PSSetShader(Shader, Instances, SizeInstance);
	}
	static void DrawIndexed(UINT IndexCount, UINT IndexStart = 0, UINT VertexStart = 0) {
		m_pContext->DrawIndexed(IndexCount, IndexStart, VertexStart);
	}
	static void SetFragConstantBuffers(UINT StartSlot, std::vector<ID3D11Buffer*> Buffers) {
		m_pContext->PSSetConstantBuffers(StartSlot, Buffers.size(), Buffers.data());
	}
	static IDXGISwapChain* GetSwapChain() { return m_pSwapChain; }
	static ID3D11Device* GetDevice() { return m_pDevice; }
	static ID3D11DeviceContext* GetContext() { return m_pContext; }
	static void Shutdown() {
		m_pContext->Release();
		m_pDevice->Release();
		m_pSwapChain->Release();
	}
private:
	static inline IDXGISwapChain* m_pSwapChain = nullptr;
	static inline ID3D11Device* m_pDevice = nullptr;
	static inline ID3D11DeviceContext* m_pContext = nullptr;
	lagDevice() = default;
	lagDevice(const lagDevice&) = delete;
	lagDevice& operator=(const lagDevice&) = delete;
	lagDevice& operator=(lagDevice&&) = delete;
	lagDevice(lagDevice&&) = delete;
	~lagDevice() {}
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
		ImGui_ImplDX11_Init(lagDevice::GetDevice(), lagDevice::GetContext());
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
/*
	This does generally the "scary" camera mathematic generating. We just need to encorporate this into the renderer. 
*/
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
	D3D11_VIEWPORT& Viewport() {
		return this->m_Viewport;
	}
	~camCamera() {

	}
public:
	D3D11_VIEWPORT m_Viewport{ };
	float m_fFOV = 60.0; // 60 is a good default Although should just make it project spec :/
	float m_fNearZ = 0.00001f;
	float m_fFarZ = 100.f;
	DirectX::XMFLOAT3 m_Position = {2,2,10}; // Target and Position can never meet. 
	DirectX::XMFLOAT3 m_Target = { 0,0,0 };
	DirectX::XMFLOAT3 m_Up = { 0,1,0 }; // Although we should change this to not be this. As up might be a global?
};
class CBackBuffer {
public:
	CBackBuffer(ID3D11Texture2D* RenderTexture = lagDevice::GetBackBuffer()) {
		HRESULT hr{S_OK};

		this->m_Backbuffer = RenderTexture;
		this->m_View = lagDevice::CreateRenderTarget(m_Backbuffer, nullptr);
		if (FAILED(hr)) {
			std::cout << "[CBackBuffer::ctor] RenderTargetView could not be created? **Check m_BackBuffer != nullptr**" << hr;
			if (m_Backbuffer == nullptr) {
				__debugbreak();
			} else {
				m_Backbuffer->Release();
			}
			return;
		}
		D3D11_TEXTURE2D_DESC depthBufferDesc{};
		m_Backbuffer->GetDesc(&depthBufferDesc);
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		m_DepthBuffer = lagDevice::CreateTexture2D(&depthBufferDesc, nullptr);
		m_DepthStencilView = lagDevice::CreateDepthStencilView(m_DepthBuffer, nullptr);

		D3D11_DEPTH_STENCIL_DESC depthstencildesc = {};
		depthstencildesc.DepthEnable = TRUE;
		depthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthstencildesc.DepthFunc = D3D11_COMPARISON_LESS;
		m_DepthState = lagDevice::CreateDepthStencilState(&depthstencildesc);
	}
	void SetTarget(ID3D11Texture2D* pTexture) {
		this->m_Backbuffer->Release();
		this->m_Backbuffer = pTexture;
	}
	void ClearViews() {
		lagDevice::ClearRenderTarget(this->m_View, fValues[0], fValues[1], fValues[2], fValues[3]);
		lagDevice::ClearDepthStencilView(this->m_DepthStencilView);
	}
	void Bind() {
		lagDevice::GetContext()->OMSetDepthStencilState(this->m_DepthState, 0);
		lagDevice::SetRenderTargets({m_View}, this->m_DepthStencilView);
	}
	ID3D11Texture2D* GetBackBuffer() {
		return this->m_Backbuffer;
	}
	ID3D11Texture2D* GetDepthBuffer() {
		return this->m_DepthBuffer;
	}
	ID3D11RenderTargetView* GetView() {
		return this->m_View;
	}
	ID3D11DepthStencilState* GetDepthState() {
		return this->m_DepthState;
	}
	ID3D11DepthStencilView* GetDepthView() {
		return this->m_DepthStencilView;
	}
	float* Values() {
		return fValues;
	}
private:
	float fValues[4]{0.40392,0.360784,0.776470,1};
	ID3D11Texture2D* m_Backbuffer = nullptr;
	ID3D11Texture2D* m_DepthBuffer;
	ID3D11RenderTargetView* m_View = nullptr;
	ID3D11DepthStencilState* m_DepthState = nullptr;
	ID3D11DepthStencilView* m_DepthStencilView = nullptr;
};
struct Vertex {
	aiVector3D POSITION;
	aiVector3D COLOR = {0,0,0};
	aiVector3D UVCOORD;
	aiVector3D NORMALS;
};
class GameCamera {
public:
	static void Init() {
		m_Camera = new camCamera();
	}
	static camCamera* GetRawCamera() {
		return m_Camera;
	}
	static void Shutdown() {
		delete m_Camera;
	}
private:
	static inline camCamera* m_Camera = nullptr;
};
class lagTexture2D {
public:
	lagTexture2D(CSTBIImage* pImage) {
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = pImage->GetWidth();
		textureDesc.Height = pImage->GetHeight();
		textureDesc.MipLevels = 1; // im not gonna abstract these yet.
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		D3D11_SUBRESOURCE_DATA textureResourceData{};
		textureResourceData.pSysMem = pImage->GetImage();
		textureResourceData.SysMemPitch = pImage->GetWidth() * sizeof(UINT);
		HRESULT hr{S_OK};
		this->pTexture = lagDevice::CreateTexture2D(&textureDesc, &textureResourceData);
		this->pTextureView = lagDevice::CreateShaderResourceView(this->pTexture);
	}
	ID3D11Texture2D* GetTexture() {
		return this->pTexture;
	}
	ID3D11ShaderResourceView* GetResource() {
		return this->pTextureView;
	}
	~lagTexture2D() {
		pTextureView->Release();
		pTexture->Release();
	}
private:
	ID3D11Texture2D* pTexture;
	ID3D11ShaderResourceView* pTextureView;
};
class CMesh {
public:
	CMesh(aiMesh* pMesh) {
		assert(sizeof(Vertex) == VtxFormat::Size() && "Size Format is incompatible. The Format and the Vertex Type must have the SAME size");
		std::vector<Vertex> Vertices;
		Vertices.reserve(pMesh->mNumVertices);
		for (int i = 0; i < pMesh->mNumVertices; i++) {
			Vertex v{};
			v.POSITION = pMesh->mVertices[i];
			v.NORMALS = pMesh->mNormals[i];
			v.UVCOORD = pMesh->mTextureCoords[0][i]; // im gonna just assume it ;/ its shitty but yk whatever. 
			printf("Cube: {%f %f %f} {%f %f %f} {%f %f %f} {%f %f %f}\n", v.POSITION.x, v.POSITION.y, v.POSITION.z, v.COLOR.x, v.COLOR.y, v.COLOR.z, v.UVCOORD.x, v.UVCOORD.y, v.UVCOORD.z, v.NORMALS.x, v.NORMALS.y, v.NORMALS.z);
			Vertices.push_back(v);
		}
		std::vector<unsigned int> mIndices{};
		mIndices.reserve(pMesh->mNumFaces * 3); // 3 per face. we just don't know yet because TRIANGULATE enforces 3 per face rule!
		for (int i = 0; i < pMesh->mNumFaces; i++) {
			auto& face = pMesh->mFaces[i];
			for (int j = 0; j < face.mNumIndices; j++) {
				mIndices.push_back(face.mIndices[j]);
			}
		}
		this->m_Buffer = new lagVertexBuffer(Vertices.data(), Vertices.size(), sizeof(Vertex));
		this->m_IdBuffer = new lagIndexBuffer(mIndices.data(), mIndices.size());
	}
	//void AppendTexture(CSTBIImage* pImage) {
	//	this->m_Textures.emplace_back(pImage);
	//}
	void Draw() {

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		lagDevice::GetContext()->IASetVertexBuffers(0, 1, this->m_Buffer->GetBufferPtr(), &stride, &offset);
		lagDevice::GetContext()->IASetIndexBuffer(this->m_IdBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
		lagDevice::DrawIndexed(m_IdBuffer->GetSize());
	}
	using VtxFormat = lagStaticVertexFormat<
		lagStaticVertexDeclaration<eStaticVertexNames::POSITION, eVertexType::VECTOR3, 0>, // POS0
		lagStaticVertexDeclaration<eStaticVertexNames::COLOR, eVertexType::VECTOR3, 0>, // COLOR0
		lagStaticVertexDeclaration<eStaticVertexNames::UVCOORD, eVertexType::VECTOR3, 0>, // UVCOORD0
		lagStaticVertexDeclaration<eStaticVertexNames::NORMALS, eVertexType::VECTOR3, 0> // NORMALS0
	>;
	lagVertexBuffer* GetVertBuffer() {
		return this->m_Buffer;
	}
	lagIndexBuffer* GetIndexBuffer() {
		return this->m_IdBuffer;
	}
	// This is much more of a type, but still it is an object :]
	VtxFormat& GetVertInfo() {
		return this->VertexInformation;
	}
	
private:
	//std::vector<lagTexture2D> m_Textures;
	VtxFormat VertexInformation;
	lagVertexBuffer* m_Buffer{nullptr};
	lagIndexBuffer* m_IdBuffer{nullptr};
};

class CTestCube {
public:
	CTestCube() {
		// --- DRAWABLE SETUP --- 
		Assimp::Importer m_Importer{};
		const aiScene* pScene = m_Importer.ReadFile("C:\\Users\\codyc\\OneDrive\\Docs from Gaming PC\\Documents\\TestModels\\cube.obj", aiPostProcessSteps::aiProcess_Triangulate | aiPostProcessSteps::aiProcess_GenNormals | aiPostProcessSteps::aiProcess_GenUVCoords | aiPostProcessSteps::aiProcess_FlipUVs);
		if (pScene->HasMeshes() && pScene->mNumMeshes == 1) {
			pMesh = new CMesh(pScene->mMeshes[0]);
		}
		m_Importer.FreeScene();
		// --- SHADER SETUP --- 
		auto vs = lagShaderCompiler::Compile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\vs.hlsl", "main", "vs_5_0", 0);
		auto ps = lagShaderCompiler::Compile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\ps.hlsl", "main", "ps_5_0", 0);
		this->m_vShader = new lagVertexShader(vs);
		this->m_Input = pMesh->GetVertInfo().CreateLayout(vs);
		this->m_fragShader = new lagFragmentShader(ps);
		// --- MISC SETUP ---
		this->m_Cbuffer = new lagConstantBuffer(); 

		CSTBIImage load{"E:\\A_Development\\Legit Engine\\Main\\Main\\rockstar_logo_64x64.png", 4};
		this->pTextures.emplace_back(&load);

		D3D11_SAMPLER_DESC SampleDesc{};
		SampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		SampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		SampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		SampleDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		SampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		SampleDesc.MipLODBias = 0.0f;
		SampleDesc.MaxAnisotropy = 1;
		SampleDesc.BorderColor[0] = SampleDesc.BorderColor[1] = SampleDesc.BorderColor[2] = SampleDesc.BorderColor[3] = 0;
		SampleDesc.MinLOD = 0;
		SampleDesc.MaxAnisotropy = D3D11_FLOAT32_MAX;
		lagDevice::GetDevice()->CreateSamplerState(&SampleDesc, &this->pSamplerState);
		lagDevice::GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //basic typical stuff.
	}
	DirectX::XMFLOAT3 POSITION{0,0,0}, ROTATION{0,2.5,0}, SCALE{1,1,1};
	void Draw() {
		Constant* Buffer = m_Cbuffer->Map();
		Constant Temp{};
		ImGui::Begin("Object");
		ImGui::InputFloat3("Position", reinterpret_cast<float*>(&POSITION));
		ImGui::InputFloat3("Rotation", reinterpret_cast<float*>(&ROTATION));
		ImGui::InputFloat3("Scale", reinterpret_cast<float*>(&SCALE));
		ImGui::End();

		DirectX::XMMATRIX mat = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&POSITION)) * DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&ROTATION)) * DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&SCALE));
		Temp.m_Model = mat;
		Temp.m_View = GameCamera::GetRawCamera()->WhatWeLookingAt();
		auto& a = GameCamera::GetRawCamera()->Viewport();
		Temp.m_Projection = GameCamera::GetRawCamera()->GetPerspectiveMtx((a.Width / a.Height));

		Temp.m_Model = DirectX::XMMatrixTranspose(Temp.m_Model);
		Temp.m_View = DirectX::XMMatrixTranspose(Temp.m_View);
		Temp.m_Projection = DirectX::XMMatrixTranspose(Temp.m_Projection);
		memcpy(Buffer, &Temp, sizeof(Constant));	
		m_Cbuffer->Unmap();
		std::vector<ID3D11ShaderResourceView*> Resources{this->pTextures.size()};
		for (int i = 0; i < pTextures.size(); i++) {
			Resources[i] = pTextures[i].GetResource();
		}

		lagDevice::GetContext()->IASetInputLayout(m_Input->GetLayout());
		lagDevice::SetFragmentShaderResources(0, Resources);
		lagDevice::SetFragmentSamplers(0, {this->pSamplerState});
		lagDevice::SetVertexConstantBuffers(0, {m_Cbuffer->GetBuffer()});
		// --- Shader Binding ---
		lagDevice::SetFragmentShader(this->m_fragShader->GetShader());
		lagDevice::SetVertexShader(this->m_vShader->GetShader());
		
		pMesh->Draw();
	}
	~CTestCube() {
		delete pMesh;
		delete m_vShader;
		delete m_fragShader;
		delete m_Cbuffer;
		delete m_Input;
	}
private:
	//Cannot exist without a mesh. 
	CMesh* pMesh{};
	lagTexture2D* pTexture;
	ID3D11SamplerState* pSamplerState;
	std::vector<lagTexture2D> pTextures;

	//runtime stuff for meshes. (also partially dependant on meshes but not entirely. For example the lagConstantBuffer is much more for "global" engine buffers same with input assembler, its dependant on shaders AND meshes, since it maps the mesh format to a readable fmt.
	lagConstantBuffer* m_Cbuffer{nullptr}; // This is pass dependant. This can tell our shaders WHERE we are rendering from more primarily. Like our perspective, however it should be interchangable, might just want to mark something that tells the computer that we are rendering with a special constant Buffer. 
	lagInputAssembler* m_Input{nullptr};
	// completely pass-dependant. i.e we could run multiple passes that would combine multiple shaders on one object. or one composite. However generally as a starter im gonna put these here.
	lagVertexShader* m_vShader{nullptr};
	lagFragmentShader* m_fragShader{nullptr}; // this is all mesh information stuff. 
};

class FullScreenPass {
public:
	static void Init(ID3D11Texture2D* Texture) {
		m_pTexture = Texture;

		m_pView = lagDevice::CreateShaderResourceView(Texture);


		pShaderForPass = new lagFragmentShader(lagShaderCompiler::Compile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\ps_full.hlsl", "PS", "ps_5_0", 0));
		auto a = lagShaderCompiler::Compile(L"E:\\A_Development\\Legit Engine\\Main\\Project1\\vs_full.hlsl", "VS", "vs_5_0", 0);
		pVShaderForPass = new lagVertexShader(a);
		InputAssembler = Fmt.CreateLayout(a);
		float quadVertices[] = {
			// positions       // flipped texCoords
			-1.0f, -1.0f,      0.0f, 1.0f,  
			-1.0f,  1.0f,      0.0f, 0.0f,  
			 1.0f, -1.0f,      1.0f, 1.0f,  

			-1.0f,  1.0f,      0.0f, 0.0f,  
			 1.0f,  1.0f,      1.0f, 0.0f,  
			 1.0f, -1.0f,      1.0f, 1.0f   
		};


		VtxBuffer = new lagVertexBuffer(quadVertices, sizeof(float) * 24, sizeof(float) * 4);

		
		D3D11_DEPTH_STENCIL_DESC DepthDesc{};
		DepthDesc.DepthEnable = false;
		DepthDesc.StencilEnable = false;
		pDepthState = lagDevice::CreateDepthStencilState(&DepthDesc);


		m_pTarget = lagDevice::CreateRenderTarget(lagDevice::GetBackBuffer(), nullptr);
	}
	static void DoPass() {
		lagDevice::GetContext()->OMSetRenderTargets(1, &m_pTarget, nullptr);
		lagDevice::ClearRenderTarget(m_pTarget, 0.5, 0, 0, 1);
		lagDevice::GetContext()->OMSetDepthStencilState(pDepthState, 0);
		lagDevice::GetContext()->IASetInputLayout(nullptr);
		UINT Stride = sizeof(float) * 4;
		UINT Offset = 0;
		lagDevice::GetContext()->IASetVertexBuffers(0, 1, VtxBuffer->GetBufferPtr(), &Stride, &Offset);
		lagDevice::GetContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		lagDevice::GetContext()->IASetInputLayout(InputAssembler->GetLayout());

		lagDevice::GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		lagDevice::GetContext()->VSSetShader(pVShaderForPass->GetShader(), nullptr, 0);
		lagDevice::GetContext()->PSSetShader(pShaderForPass->GetShader(), nullptr, 0);
		

		lagDevice::GetContext()->PSSetShaderResources(0, 1, &m_pView);
		
		lagDevice::GetContext()->Draw(6, 0);
	}
	static void Shutdown() {

	}
private:
	using Format = lagStaticVertexFormat<lagStaticVertexDeclaration<eStaticVertexNames::POSITION, eVertexType::VECTOR2, 0>, lagStaticVertexDeclaration<eStaticVertexNames::UVCOORD, eVertexType::VECTOR2, 0>>;
	static inline lagInputAssembler* InputAssembler{nullptr};
	static inline lagVertexBuffer* VtxBuffer{nullptr};
	static inline Format Fmt{};
	static inline ID3D11Texture2D* pMyTexture = nullptr;
	static inline ID3D11DepthStencilState* pDepthState = nullptr;
	static inline ID3D11RenderTargetView* m_pTarget{nullptr};
	static inline ID3D11Texture2D* m_pTexture = nullptr;
	static inline ID3D11ShaderResourceView* m_pView;
	static inline lagFragmentShader* pShaderForPass = nullptr;
	static inline lagVertexShader* pVShaderForPass = nullptr;
};
class CRenderEntityPass {
public:
	CRenderEntityPass() {
		m_Cube = new CTestCube();
		//m_BackBuffer = new CBackBuffer();

		D3D11_TEXTURE2D_DESC Desc{};
		lagDevice::GetBackBuffer()->GetDesc(&Desc);
		Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

		pRTT = lagDevice::CreateTexture2D(&Desc, nullptr);
		pRTV = lagDevice::CreateRenderTarget(pRTT, nullptr);

		Desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		Desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		pDST = lagDevice::CreateTexture2D(&Desc, nullptr);
		pDSV = lagDevice::CreateDepthStencilView(pDST, nullptr);
		
		D3D11_DEPTH_STENCIL_DESC StencilDesc = {};
		StencilDesc.DepthEnable = TRUE;
		StencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		StencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		
		pDSS = lagDevice::CreateDepthStencilState(&StencilDesc);

		HRESULT hr{};
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		RECT rc{};
		GetClientRect(CMainWindow::Get().GetRawWindow()->GetWindowHandle(), &rc);
		viewport.Width = rc.right - rc.left;
		viewport.Height = rc.bottom - rc.top;
		GameCamera::GetRawCamera()->Viewport() = viewport;

		D3D11_RASTERIZER_DESC rastDesc{};
		rastDesc.FillMode = D3D11_FILL_SOLID;
		rastDesc.CullMode = D3D11_CULL_BACK; // Disable culling to avoid orientation issues
		rastDesc.DepthClipEnable = TRUE;

		hr = lagDevice::GetDevice()->CreateRasterizerState(&rastDesc, &rastState);
		lagDevice::GetContext()->RSSetState(rastState);
	}
	ID3D11Texture2D* GetTexture() {
		return this->pRTT;
	}
	void SetRender() {
		lagDevice::SetRenderTargets({this->pRTV}, pDSV);
		lagDevice::GetContext()->OMSetDepthStencilState(pDSS, 0);
	}
	void DoPass() {
		lagDevice::GetContext()->ClearState(); // Clear the State before the Frame
		lagDevice::SetRenderTargets({this->pRTV}, pDSV);
		lagDevice::ClearRenderTarget(pRTV, 1,1,1,1);
		lagDevice::ClearDepthStencilView(this->pDSV);
		lagDevice::GetContext()->OMSetDepthStencilState(pDSS, 0);
		lagDevice::SetViewports({viewport});
		lagDevice::GetContext()->RSSetState(rastState);
		lagDevice::GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_Cube->Draw();
		//ImGui::Begin("Renderer");
		//if (ImGui::CollapsingHeader("BackBuffer Config")) {
		//	ImGui::ColorEdit4("Color", m_BackBuffer->Values());
		//	//int Item = CurItem;
		//	//ImGui::Combo("Output Merger View", &Item, this->m_OM, 2);
		//	//if (Item != CurItem) {
		//	//	lagDevice::SetRenderTargets({m_Views[Item]}, this->m_BackBuffer->GetDepthView());
		//	//	CurItem = Item;
		//	//}
		//}
		//ImGui::End();
	}
	~CRenderEntityPass() {
		//delete m_BackBuffer;
		delete m_Cube;
	}
	CTestCube& GetCube() {
		return *m_Cube;
	}
private:
	int CurItem{};
	ID3D11RasterizerState* rastState = nullptr;
	

	ID3D11Texture2D* pRTT = nullptr;
	ID3D11Texture2D* pDST = nullptr;
	ID3D11RenderTargetView* pRTV = nullptr;
	ID3D11DepthStencilState* pDSS = nullptr;
	ID3D11DepthStencilView* pDSV = nullptr;

	CTestCube* m_Cube;
	D3D11_VIEWPORT viewport;

};
class CRenderer {
public:
	static CRenderer& Get() {
		static CRenderer s_Renderer;
		return s_Renderer;
	}
	void InitDeviceLayer() {
		//GetDevice = __GetDevice;
		//GetContext = __GetContext;
		lagShaderCompiler::InitClass();
	}

	void SetupConstantBuffer() {
	}
	void UpdateCameraPosition() {
	}
	void Init() {
		lagDevice::Init(wind->GetWindowHandle());
		InitDeviceLayer();
		GameCamera::Init();

		//SetupConstantBuffer();
		//if (FAILED(hr)) {
		//	std::cout << "[BUFFER]" << hr << std::endl;
		//	return;
		//}
		setup();
	} 

	void LegacySetup() {
		::GetDevice = lagDevice::GetDevice;
		::GetContext = lagDevice::GetContext;
	}
	void setup() {
		LegacySetup();
		m_pEntityPass = new CRenderEntityPass();
		FullScreenPass::Init(m_pEntityPass->GetTexture());
		//D3D11_TEXTURE2D_DESC Desc{};
		//m_BackBuffer->GetDepthBuffer()->GetDesc(&Desc);
		//Desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		//Desc.Format = DXGI_FORMAT_R32_FLOAT;
		//DepthTexture = lagDevice::CreateTexture2D(&Desc, nullptr);
		//FullScreenPass::Init(DepthTexture);
		//v = lagDevice::CreateRenderTarget(DepthTexture, nullptr);
		//DepthBufferShaderRes = lagDevice::CreateShaderResourceView(DepthTexture);
	}
	void Clear() {
		
	}
	std::vector<ID3D11RenderTargetView*> m_Views{2};
	const char* m_OM[2] = {"Normal", "Depth"};
	int CurItem = 0;
	//ID3D11Texture2D* DepthTexture{};
	//ID3D11ShaderResourceView* DepthBufferShaderRes{};
	//ID3D11RenderTargetView* v{};
	//bool HasBound = false;
	bool b = false;
	void Update() {
		m_pEntityPass->DoPass();
		FullScreenPass::DoPass();
		//if (ImGui::Begin("Fuck")) {
		//	if (ImGui::Checkbox("Label", &b)) {
		//		if (b) {  }
		//		else {
		//			m_pEntityPass->SetRender();
		//		};
		//	}
		//	ImGui::End();
		//}
	}
	void Present() {
		lagDevice::Present(1, 0);
	}
	void Shutdown() {
		delete m_pEntityPass;
		GameCamera::Shutdown();
		lagShaderCompiler::DestroyClass();
		lagDevice::Shutdown();
	}
private:
	CRenderEntityPass* m_pEntityPass = nullptr;
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
		CRenderer::Get().Init();
		m_Debug = new CDebug();
#else
		printf("[Error]: Unknown/Unanticipated Platform. We are unable to support your operating system or platform.");
		this->m_bStopRunning = true;
#endif // LE_WIN
		return;
	}
	void Update() {

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
		CLogger::Shutdown();
	}
private:
	CDebug* m_Debug = nullptr;
#ifdef LE_WIN32
	CWinArgs*
#else 
	legit::fwCmdArgs*
#endif
	m_Arguments = nullptr;
	bool m_bStopRunning = false;
};
#endif // USE_RENDERER