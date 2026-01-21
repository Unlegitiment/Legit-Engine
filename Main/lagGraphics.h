#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <vector>
struct lagByteCode {
	std::vector<char> Byte;
};
class lagShaderCompiler {
public:
	lagByteCode Compile(const wchar_t* ShaderPath, const char* ShaderEntry, const char* ShaderVersion) {
		ID3DBlob* pBlob, * pErr{};
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

using lagDepthView = ID3D11DepthStencilView;
using lagDepthState = ID3D11DepthStencilState;
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
	static D3D11_VIEWPORT GetClientViewport(HWND Window) {
		D3D11_VIEWPORT Port{};
		Port.TopLeftX = 0;
		Port.TopLeftY = 0;
		RECT rc{};
		GetClientRect(Window, &rc);
		Port.Width = rc.right - rc.left;
		Port.Height = rc.bottom - rc.top;
		return Port;
	}
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
	static lagDeviceContext* GetDeviceContext() {
		return sm_pDeviceContext;
	}
	static lagDeviceHandle* GetDeviceHandle() {
		return sm_pDevice;
	}
	static lagSwapChain* GetSwapChain() {
		return sm_pSwapChain;
	}
	static void SetRenderTargets(std::vector<lagRenderTarget*> pTargets, ID3D11DepthStencilView* DepthStencil) {
		sm_pDeviceContext->OMSetRenderTargets(pTargets.size(), pTargets.data(), DepthStencil);
	}
	static void ClearRenderTarget(lagRenderTarget* pTarget, float* Color) {
		sm_pDeviceContext->ClearRenderTargetView(pTarget, Color);
	}
	static void ClearDepthView(lagDepthView* pView, UINT ClearFlags, FLOAT Depth, UINT8 Stencil) {
		sm_pDeviceContext->ClearDepthStencilView(pView, ClearFlags, Depth, Stencil);
	}
	static void SetDepthState(lagDepthState* pState, UINT StencilRef) {
		sm_pDeviceContext->OMSetDepthStencilState(pState, StencilRef);
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
	static lagDepthState* CreateDepthState(const D3D11_DEPTH_STENCIL_DESC* pDesc) {
		lagDepthState* pState{nullptr};
		HRESULT hr = sm_pDevice->CreateDepthStencilState(pDesc, &pState);
		if (FAILED(hr)) {
			return nullptr;
		}
		return pState;
	}
	static lagDepthView* CreateDepthView(lagResource* pRes, D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc) {
		lagDepthView* DSV{nullptr};
		HRESULT hr = sm_pDevice->CreateDepthStencilView(pRes, pDesc, &DSV);
		if (FAILED(hr)) {
			return nullptr;
		}
		return DSV;
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
