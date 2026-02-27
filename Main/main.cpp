#include "stb_image.h"
#define NOMINMAX
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <LECore/headers/platform_specs.h>
#include "input/ioInput.h"
#include <LITemplates/alloc/Default.h>
using namespace legit;
#ifdef LE_WIN
#include <Windows.h>
#include "lagGraphics.h"
#include <ctime>
#include <unordered_map>
#include <LITemplates/pointers/Auto.h>
class CWindowCore {
public:
	struct sWindowInformation {
		bool IsCloseRequested = false;
		bool IsQuitRequested = false;
		bool WasWindowJustCreated;
		unsigned long WindowWidth, WindowHeight;
	};
	static inline sWindowInformation* sm_pWindowMessage{nullptr};
};
class CWindowMain : public CWindowCore {
public:
	static void Init() {
		InitWindow();
	}
	static void Update() {
		UpdateWndProc();
	}
	static void Shutdown() {
		ShutdownWindow();
	}
	static HWND GetWindow() {
		return WINDOW;
	}
private:
	static inline std::vector<WNDPROC>* WindowProcesses = nullptr;
	static inline wchar_t WINDOW_CLASS[] = L"lagWindowClass";
	static LRESULT CALLBACK GameWindowsProcedure(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_NCCREATE:
				sm_pWindowMessage->WasWindowJustCreated = true;
				break;
			case WM_CREATE:
				{
					CREATESTRUCT* pStruct = (CREATESTRUCT*)lParam;
					sm_pWindowMessage->WindowHeight = pStruct->cy;
					sm_pWindowMessage->WindowWidth = pStruct->cx;
					sm_pWindowMessage->WasWindowJustCreated = false;
					break;
				}
			case WM_CLOSE:
				{
					sm_pWindowMessage->IsCloseRequested = true;
					int res = MessageBox(NULL, L"Would you like to quit?", L"Quit?", MB_YESNO);
					if (res == IDYES) {
						sm_pWindowMessage->IsQuitRequested = true;
					}
					return 0;
				}
			case WM_SIZE:
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);
				sm_pWindowMessage->WindowWidth = width;
				sm_pWindowMessage->WindowHeight = height;
				break;
		}
		for (const auto func : *WindowProcesses) {
			auto res = func(window, msg, wParam, lParam);
			if (res) {
				return res;
			}
		}
		return DefWindowProc(window, msg, wParam, lParam);
	}
	static inline HBRUSH IDontWantToBlindMySelfEveryTimeThanks{nullptr};
	static void InitWindow() {
		WindowProcesses = new std::vector<WNDPROC>();
		WNDCLASSW winClass{};
		winClass.lpszClassName = WINDOW_CLASS;
		winClass.lpfnWndProc = GameWindowsProcedure;
		winClass.hInstance = GetModuleHandle(NULL);

		IDontWantToBlindMySelfEveryTimeThanks = CreateSolidBrush(RGB(0, 0, 0));
		winClass.hbrBackground = IDontWantToBlindMySelfEveryTimeThanks;

		RegisterClass(&winClass);
		sm_pWindowMessage = new sWindowInformation();
		WINDOW = CreateWindowExW(NULL, WINDOW_CLASS, L"WindowName", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);
		ShowWindow(WINDOW, SW_SHOW);
	}
public:
	static void AddToWindowsHandler(WNDPROC p) {
		if (!WindowProcesses) return;
		WindowProcesses->push_back(p);
	}
private:

	static void UpdateWndProc() {
		MSG m{};
		while (PeekMessage(&m, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&m);
			DispatchMessageW(&m);
		}

	}
	static void ShutdownWindow() {
		CloseWindow(WINDOW);
		WINDOW = nullptr;
		DeleteObject(IDontWantToBlindMySelfEveryTimeThanks);
		legit::Delete(sm_pWindowMessage); // 
		legit::Delete(WindowProcesses);
	}
	static inline HWND WINDOW{nullptr};
};
class InputElements {
public:
	InputElements(){}
	~InputElements(){}
	void AddElement(const char* SemanticName, int SemanticIndex, DXGI_FORMAT Format, unsigned int SizeOfFormat) {
		D3D11_INPUT_ELEMENT_DESC Desc{};
		Desc.SemanticName = SemanticName;
		Desc.SemanticIndex = SemanticIndex;
		Desc.Format = Format;
		Desc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		m_Descs.push_back(Desc);
		Stride += SizeOfFormat;
	}
	int GetStride() const{
		return this->Stride;
	}
	int GetByteWidth(unsigned long SizeOf) const {
		return Stride / SizeOf;
	}
	int GetOffset() const{
		return this->Offset;
	}
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& GetDescr() {
		return m_Descs;
	}
private:
	std::vector<D3D11_INPUT_ELEMENT_DESC> m_Descs{};
	int Offset = 0;
	int Stride = 0;
};
class CMesh {
public:
	CMesh(std::vector<float> Vertices, std::vector<int> Indices, InputElements Elements) : m_Elements(Elements){
		if (Vertices.empty()) return; // you can't create a fucking mesh with literally 0 data. thats not how meshes work. 
		D3D11_BUFFER_DESC VertDesc{};
		VertDesc.ByteWidth = sizeof(float) * Vertices.size();
		VertDesc.Usage = D3D11_USAGE_IMMUTABLE;
		VertDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA VertData{};
		VertData.pSysMem = Vertices.data();
		this->NumberOfVertices = Vertices.size();
		m_pVertexBuffer = lagGraphics::CreateBuffer(&VertDesc, &VertData);
		if (!Indices.empty()) {
			D3D11_BUFFER_DESC IndexDesc{};
			IndexDesc.ByteWidth = sizeof(int) * Indices.size();
			IndexDesc.Usage = D3D11_USAGE_IMMUTABLE;
			IndexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			D3D11_SUBRESOURCE_DATA IndexData{};
			IndexData.pSysMem = Indices.data();
			this->NumberOfIndices = Indices.size();
			m_pIndexBuffer = lagGraphics::CreateBuffer(&IndexDesc, &IndexData);
		}
	}
	lagBuffer* GetVertexBuffer() {
		return this->m_pVertexBuffer;
	}
	lagBuffer* GetIndexBuffer() {
		return this->m_pIndexBuffer;
	}
	InputElements& GetInput() {
		return this->m_Elements;
	}
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& GetElements() {
		return m_Elements.GetDescr();
	}
	void Draw() {
		unsigned int offsets = GetInput().GetOffset();
		unsigned int stride = GetInput().GetStride(); // like obviously I know how many elements there are. but how does one know the byte stride?
		lagGraphics::SetPrimitiveTopology(GetTopology());
		lagGraphics::SetVertexBuffers(0, {m_pVertexBuffer}, &stride, &offsets);
		lagGraphics::SetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		if (m_pIndexBuffer) {
			lagGraphics::DrawIndexed(NumberOfIndices, 0, 0);
		} else {
			lagGraphics::Draw(NumberOfVertices / GetInput().GetByteWidth(sizeof(float)), 0);
		}
	}
	~CMesh() {
		if (m_pVertexBuffer)
			m_pVertexBuffer->Release();
		if (m_pIndexBuffer)
			m_pIndexBuffer->Release();
	}
	lagPrimitiveTopology GetTopology() {
		return lagPrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; // hardcoded.
	}
private:
	lagBuffer* m_pVertexBuffer{nullptr}; // see so the problem is that, this is weird lmao. so we store the buffers on the GPU and we store the Elements on the CPU. The mental model is strange, but it works? Idk.
	lagBuffer* m_pIndexBuffer{nullptr};
	InputElements m_Elements{};
	unsigned int NumberOfVertices = 0;
	unsigned int NumberOfIndices = 0;
};
class buffer {
public:
	static lagBuffer* CreateBuffer(D3D11_USAGE Usage, D3D11_BIND_FLAG BindFlags, size_t ByteStride, const void* Data) {
		D3D11_BUFFER_DESC Desc{};
		Desc.ByteWidth = ByteStride;
		Desc.Usage = Usage;
		Desc.BindFlags = BindFlags;
		D3D11_SUBRESOURCE_DATA mData{};
		mData.pSysMem = Data;
		return lagGraphics::CreateBuffer(&Desc, &mData);
	}
	static lagBuffer* CreateBuffer(D3D11_USAGE Usage, D3D11_BIND_FLAG BindFlags, size_t ByteStride, D3D11_CPU_ACCESS_FLAG cpuaccess,  const void* Data) {
		D3D11_BUFFER_DESC Desc{};
		Desc.ByteWidth = ByteStride;
		Desc.Usage = Usage;
		Desc.BindFlags = BindFlags;
		Desc.CPUAccessFlags = cpuaccess;
		D3D11_SUBRESOURCE_DATA mData{};
		mData.pSysMem = Data;
		return lagGraphics::CreateBuffer(&Desc, &mData);
	}
private:

};
namespace legit {
	template<typename T, typename R> constexpr T StaticCast(R Object) {
		return static_cast<T>(Object);
	}
	template<typename T, typename R> constexpr T* ReinterpretCast(R* Object) {
		return reinterpret_cast<T*>(Object);
	}
}
template<typename T>
class lagMapSegment {
public:
	lagMapSegment(lagBuffer* Buffer) {
		m_pFetchedPointer = legit::StaticCast<T*>(lagGraphics::Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0));
		this->m_Buffer = Buffer;
	}
	T* operator->() { return m_pFetchedPointer; }
	T& operator*() { return *m_pFetchedPointer; }
	T* Get() {
		return this->m_pFetchedPointer;
	}
	~lagMapSegment() {
		lagGraphics::Unmap(m_Buffer, 0);
		m_pFetchedPointer = nullptr;
	}
private:
	T* m_pFetchedPointer = nullptr;
	lagBuffer* m_Buffer = nullptr;
};
template<typename T>
class lagBufferUpdate {
public:
	lagBufferUpdate(lagBuffer* gpuBuf) : m_Buffer(gpuBuf){}
	lagBuffer* Buffer() {
		return this->m_Buffer;
	}
	void Update(const T& CopyValues) {
		lagMapSegment<T> seg = (m_Buffer);
		*seg = CopyValues;
	}
	~lagBufferUpdate() {}
private:
	lagBuffer* m_Buffer;
};

class BlinnPhongEffect {
public:
	BlinnPhongEffect(InputElements& m_Elements, ID3DBlob& pVSBlob, ID3DBlob& pPSBlob) {
		FragmentShader(pPSBlob);
		auto res = Vertex(pVSBlob);
		m_InputAssembler = lagGraphics::CreateInputAssembler(m_Elements.GetDescr(), res);
		m_VTConsts.push_back(buffer::CreateBuffer(D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, sizeof(LagMatrices), D3D11_CPU_ACCESS_WRITE, &DefaultMatrices));
		m_FSConsts.push_back(buffer::CreateBuffer(D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, sizeof(LagColorLighting), D3D11_CPU_ACCESS_WRITE, &LightingBuffer));
	}
	void Draw(CMesh* pMesh) {
		lagGraphics::SetInputAssembler(m_InputAssembler);
		lagGraphics::SetVertexShader(m_pVertexShader);
		lagGraphics::SetFragmentShader(m_pFragmentShader);
		UpdateSubresource();
		lagGraphics::SetVertexShaderBuffers(0, m_VTConsts); // vertex shader. 
		lagGraphics::SetFragmentShaderBuffers(0, m_FSConsts); // frag
		pMesh->Draw();
	}
	static inline DirectX::XMVECTOR VIEWPOS = {2,2,-3};
private:
	void UpdateSubresource() {
		lagBufferUpdate<LagMatrices> mats{this->m_VTConsts[0]};
		LagMatrices mat{};
		mat.m_Model = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
		mat.m_Projection = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.f), 16.0 / 9.0, 0.01f, 100));
		mat.m_View = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(VIEWPOS, {0,0,0}, {0,1,0}));
		mats.Update(mat);
		lagBufferUpdate<LagColorLighting> Lighting{this->m_FSConsts[0]};
		LagColorLighting LightingParams{};
		LightingParams.m_ObjectColor[0] = 1.0;
		LightingParams.m_ObjectColor[1] = 0.5f;
		LightingParams.m_ObjectColor[2] = 0.31f;
		LightingParams.m_ObjectColor[3] = 1;
		LightingParams.m_LightColor[0] = 1;
		LightingParams.m_LightColor[1] = 1;
		LightingParams.m_LightColor[2] = 1;
		LightingParams.m_LightColor[3] = 1;
		LightingParams.m_LightPosition[0] = 2;
		LightingParams.m_LightPosition[1] = 2;
		LightingParams.m_LightPosition[2] = -2;
		LightingParams.m_LightPosition[3] = 0;
		LightingParams.m_ViewPosition[0] = VIEWPOS.m128_f32[0];
		LightingParams.m_ViewPosition[1] = VIEWPOS.m128_f32[1];
		LightingParams.m_ViewPosition[2] = VIEWPOS.m128_f32[2];
		LightingParams.m_ViewPosition[3] = VIEWPOS.m128_f32[3]; 
		Lighting.Update(LightingParams);
	}
	void FragmentShader(ID3DBlob& pPSBlob) {
		std::vector<char> m_Byte{};
		m_Byte.resize(pPSBlob.GetBufferSize());
		memcpy(m_Byte.data(), pPSBlob.GetBufferPointer(), pPSBlob.GetBufferSize());
		m_pFragmentShader = lagGraphics::CreateFragmentShader(m_Byte);
	}
	std::vector<char> Vertex(ID3DBlob& pVSBlob) {
		std::vector<char> m_Byte{};
		m_Byte.resize(pVSBlob.GetBufferSize());
		memcpy(m_Byte.data(), pVSBlob.GetBufferPointer(), pVSBlob.GetBufferSize());
		m_pVertexShader = lagGraphics::CreateVertexShader(m_Byte);
		return m_Byte;
	}
	struct LagMatrices {
		DirectX::XMMATRIX m_Projection; // 
		DirectX::XMMATRIX m_View; // shifts universe to camera
		DirectX::XMMATRIX m_Model; // looks  
	} DefaultMatrices{};
	struct alignas(16) LagColorLighting {
		float m_ObjectColor[4];
		float m_LightColor[4];
		float m_LightPosition[4];
		float m_ViewPosition[4];
	} LightingBuffer{};
	std::vector<lagBuffer*> m_VTConsts{}; 
	std::vector<lagBuffer*> m_FSConsts{};
	lagInputAssembler* m_InputAssembler{nullptr};
	lagVertexShader* m_pVertexShader{nullptr};
	lagFragmentShader* m_pFragmentShader{nullptr};
};

class RenderPassMesh {
public:
	static void Shaders() {
		const wchar_t* Path = L"E:\\A_Development\\Legit Engine\\Main\\Project1\\simple_vert_shader.hlsl";
		ID3DBlob* pVSBlob = nullptr, * pPSBlob = nullptr, * pErr = nullptr;
		auto res = D3DCompileFromFile(Path, nullptr, nullptr, "VS_MainQuad", "vs_5_0", 0, 0, &pVSBlob, &pErr);
		if (FAILED(res)) {
			printf("Shader Compiler Failed: %s", (char*)pErr->GetBufferPointer());
			__debugbreak();
		}
		res = D3DCompileFromFile(Path, nullptr, nullptr, "PS_PhongBasic", "ps_5_0", 0, 0, &pPSBlob, &pErr);
		if (FAILED(res)) {
			printf("Shader Compiler Failed: %s", (char*)pErr->GetBufferPointer());
			__debugbreak();
		}
		sm_pEffect = new BlinnPhongEffect(sm_pMesh->GetInput(), *pVSBlob, *pPSBlob);
	}
	/*
		This part is not really tied to the Shader, You can create the definitions whereever. But the problem arises when you want to do more than this lmao.
		I think that this is the hard part with D3D11, is that the line is often blurred as to like how operations are used in tandem.
	*/
	static void Init() {
		/*
			A lot of big stuff is removed by the context of this call. I'd consider a new approach to Initializing the procedure or introducing a middle-man between the two layers.
		*/
		sm_pRenderTarget = lagGraphics::CreateRenderTarget(lagGraphics::GetBackBuffer());
		lagGraphics::SetRenderTargets({sm_pRenderTarget}, nullptr);
		std::vector<float> Vert{};
		Assimp::Importer Importer{};
		std::vector<int> Indices{};
		const aiScene* scene = Importer.ReadFile("E:\\A_Development\\Legit Engine\\Main\\Main\\assets_\\cube.obj", aiProcess_Triangulate | aiProcess_GenNormals);
		for (int i = 0; i < scene->mNumMeshes; i++) {
			auto* mesh = scene->mMeshes[i];
			for (int j = 0; j < mesh->mNumVertices; j++) {
				Vert.emplace_back(mesh->mVertices[j].x);
				Vert.emplace_back(mesh->mVertices[j].y);
				Vert.emplace_back(mesh->mVertices[j].z);
				Vert.emplace_back(mesh->mNormals[j].x);
				Vert.emplace_back(mesh->mNormals[j].y);
				Vert.emplace_back(mesh->mNormals[j].z);
			}
			for (int j = 0; j < mesh->mNumFaces; j++) {
				aiFace& face = mesh->mFaces[j];
				if (face.mNumIndices != 3) {
					printf("Wrong Num of Indices per-face. Geometry is likely fucked!\n");
				}
				Indices.push_back(face.mIndices[0]);
				Indices.push_back(face.mIndices[1]);
				Indices.push_back(face.mIndices[2]);
			}
		}
		Importer.FreeScene();
		sm_pMesh = new CMesh(Vert, Indices, {});
		sm_pMesh->GetInput().AddElement("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, sizeof(float) * 3);
		sm_pMesh->GetInput().AddElement("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, sizeof(float) * 3);
		Shaders();
		D3D11_RASTERIZER_DESC RasterDesc{};
		RasterDesc.CullMode = D3D11_CULL_BACK;
		RasterDesc.FillMode = D3D11_FILL_SOLID;
		RasterDesc.DepthClipEnable = 1;
		sm_pRasterstate = lagGraphics::CreateRasterizationState(&RasterDesc);
	}
	static inline lagRasterizerState* sm_pRasterstate{nullptr};
	static inline std::vector<lagBuffer*> sm_pBuffers{};
	static void Render() {
		float fColor[4] = {0,0,0,1};
		lagGraphics::ClearRenderTarget(sm_pRenderTarget, fColor);
		lagGraphics::SetRasterizerState(sm_pRasterstate);
		D3D11_VIEWPORT port{};
		port.Width = CWindowCore::sm_pWindowMessage->WindowWidth;
		port.Height = CWindowCore::sm_pWindowMessage->WindowHeight;
		lagGraphics::SetViewports({port});
		sm_pEffect->Draw(sm_pMesh);
	}
	static void Shutdown() {

	}
private:
	static inline BlinnPhongEffect* sm_pEffect{nullptr};
	static inline CMesh* sm_pMesh{nullptr};
	static inline lagRenderTarget* sm_pRenderTarget = nullptr;
};
class CApplication {
public:
	static void Init() {
		CWindowMain::Init();
		legit::ioInput::Init(CWindowMain::AddToWindowsHandler);
		lagGraphics::Init(CWindowMain::GetWindow());
		RenderPassMesh::Init();
	}
	static bool Update() {
		while (!sm_bCanClose) {
			CWindowMain::Update(); // process window messages. 
			if (CWindowMain::sm_pWindowMessage->IsCloseRequested && CWindowMain::sm_pWindowMessage->IsQuitRequested) {
				sm_bCanClose = true;
			}
			RenderPassMesh::Render();
			lagGraphics::Present(1, 0);
		}
		return true;
	}
	static void Shutdown() {
		RenderPassMesh::Shutdown();
		lagGraphics::Shutdown();
		legit::ioInput::Shutdown();
		CWindowMain::Shutdown();
	}
private:
	static inline bool sm_bCanClose = false;
private:
};


int main(int argc, char** argv) {
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