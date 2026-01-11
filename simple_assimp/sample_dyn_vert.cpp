#include <stdio.h>
#include <iostream>
#include <DirectXMath.h>
#include <dxgiformat.h>
#include <unordered_set>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <DbgHelp.h>
#include <varargs.h>

#pragma comment(lib, "dbghelp.lib")
#include "../LECore/le_types.h"
#include "../LECore/headers/platform_specs.h"
namespace legit {
	enum class eLogLevel {
		INFO,
		WARNING,
		ERRORF,
		FATAL,
		TRACE,
		SIZEOF_LOG_LVL
	};
	template<legit::u64 Size = 1028> class lagLogger {
	private:
		static constexpr const char* LevelToColor[] = {
			RNorm,
			RYellow,
			RHIRed,
			RRed,
			RHIPurple,
		};
		static constexpr const char* LevelToName[] = {
			"INFO",
			"WARN",
			"ERROR",
			"FATAL",
			"TRACE",
		};
	public:
		static constexpr legit::u64 SizeOfBuffer = Size;
		char* Formatf(const char* fmt, ...) {
			va_list list;
			va_start(list, fmt);
			char* buffer = new char[1028]{0};
			vsnprintf_s(buffer, sizeof(buffer), 1028, fmt, list);
			va_end(list);
			return buffer;
		}
		using lagDebugFunctor = void(*)(eLogLevel lvl, const char* msg);
		static void DefaultPrintFuncLag(eLogLevel lvl, const char* msg) {
			printf("%s[%s] %s\n", LevelToColor[(legit::u8)lvl], LevelToName[(legit::u8)lvl], msg);
		}
		lagDebugFunctor Print = DefaultPrintFuncLag;

		lagLogger(bool Sample = false) {
			if (Sample) {
				EnableLayer();
				this->Println("This is a Sample");
				this->Warnf("This is a Sample");
				this->Errorf("This is a Sample");
				this->Fatalf("This is a Sample");
				DisableLayer();
			}
		}
		void EnableLayer() {
			m_bIsDbgLayerEnabled = true;
		}
		void DisableLayer() {
			m_bIsDbgLayerEnabled = false;
		}
		void Println(const char* fmt, ...) {
			if (!m_bIsDbgLayerEnabled) return;
			va_list list;
			va_start(list, fmt);
			char buffer[Size]{0};
			vsnprintf_s(buffer, sizeof(buffer), fmt, list);
			va_end(list);
			Print(eLogLevel::INFO, buffer);
		}
		void Warnf(const char* fmt, ...) {
			if (!m_bIsDbgLayerEnabled) return;
			va_list list;
			va_start(list, fmt);
			char buffer[Size]{0};
			vsnprintf_s(buffer, sizeof(buffer), fmt, list);
			va_end(list);
			Print(eLogLevel::WARNING, buffer);
		}
		void Errorf(const char* fmt, ...) {
			if (!m_bIsDbgLayerEnabled) return;
			va_list list;
			va_start(list, fmt);
			char buffer[Size]{0};
			vsnprintf_s(buffer, sizeof(buffer), fmt, list);
			va_end(list);
			Print(eLogLevel::ERRORF, buffer);
		}
		void Fatalf(const char* fmt, ...) {
			if (!m_bIsDbgLayerEnabled) return;
			va_list list;
			va_start(list, fmt);
			char buffer[Size]{0};
			vsnprintf_s(buffer, sizeof(buffer), fmt, list);
			va_end(list);
			Print(eLogLevel::FATAL, buffer);
		}
		void Stacktrace(legit::u32 iSkipFrames) {
#if LE_WIN
			size_t Buf[32]{};
			short sCaptured = CaptureStackBackTrace(iSkipFrames, 32, (void**)Buf, NULL);

			HANDLE Proc = GetCurrentProcess(); // GetCurrentProcess Windows Function. 

			SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256, 1);
			symbol->MaxNameLen = 255;
			symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
			for (int i = 0; i < sCaptured; i++) {
				SymFromAddr(Proc, (DWORD64)(Buf[i]), 0, symbol);
				char buffer[1028]{0};
				sprintf_s(buffer, "%s - 0x%p", symbol->Name, symbol->Address);
				Print(eLogLevel::TRACE, buffer);
			}
			free(symbol);
#endif
		}
		~lagLogger() {

		}
	private:
		bool m_bIsDbgLayerEnabled = false;
	};
	class lagDebugger {
	public:
		static void Init() {
			lagDebugger::m_Logger = new lagLogger<>();
			m_Logger->EnableLayer();
			Enabled = true;
		}
		static lagLogger<>* GetLogger() {
			return m_Logger;
		}
#define FuncDefLog(x) static void x(const char* fmt, ...) { va_list list; va_start(list,fmt); char buffer[lagLogger<>::SizeOfBuffer]{0}; vsnprintf_s(buffer,sizeof(buffer), fmt, list); va_end(list); GetLogger()->x("%s", buffer);}
		FuncDefLog(Println);
		FuncDefLog(Warnf);
		FuncDefLog(Errorf);
		static void Fatalf(const char* fmt, ...) {
			va_list list;
			va_start(list, fmt);
			char buffer[lagLogger<>::SizeOfBuffer]{0};
			vsnprintf_s(buffer, sizeof(buffer), fmt, list);
			va_end(list);
			GetLogger()->Fatalf("%s", buffer);
			GetLogger()->Stacktrace(2);
		};
#undef FuncDefLog
		static void Destroy() {
			delete m_Logger;
			Enabled = false;
		}
		static bool IsEnabled() {
			return Enabled;
		}
		static bool IsDebugEnabled() {
			
		}
	private:
		static inline lagLogger<>* m_Logger{};
		static inline bool Enabled = true;
	};
} // namespace legit;

class CLogger {
	struct Globals {
		bool ActiveLevels[(int)legit::eLogLevel::SIZEOF_LOG_LVL];
		bool IsDebugEnabled = _DEBUG ? 1 : 0;
	} static inline sm;
public:
	static constexpr int LOG_BUFFER_SIZE = 1028u;
	static void Init(bool DebugOverride = false) {
		sm.ActiveLevels[0] = sm.ActiveLevels[1] = sm.ActiveLevels[2] = sm.ActiveLevels[3] = false;
		if (DebugOverride) {
			sm.IsDebugEnabled = true;
		}
		SymInitialize(GetCurrentProcess(), NULL, true);
	}
	static void Shutdown() {
		SymCleanup(GetCurrentProcess());
	}
	static void EnableLoggingLayer(legit::eLogLevel layer) {
		sm.ActiveLevels[(int)layer] = true;
	}
	static void DisableLoggingLayer(legit::eLogLevel layer) {
		sm.ActiveLevels[(int)layer] = false;
	}
	static void Println(const char* fmt, ...) { // Standard input/output.
		if (!sm.IsDebugEnabled) return;
		va_list list{};
		va_start(list, fmt);
		char Buffer[1028]{};
		vsnprintf_s(Buffer, sizeof(Buffer), fmt, list);
		va_end(list);
		printf("%s", Buffer);
	}
	static void PrintStackTrace(int frametoSkip = 1) {
		if (!sm.IsDebugEnabled) return;
		size_t Buf[32]{};
		short sCaptured = CaptureStackBackTrace(frametoSkip, 32, (void**)Buf, NULL);

		HANDLE Proc = GetCurrentProcess();

		SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256, 1);
		symbol->MaxNameLen = 255;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		for (int i = 0; i < sCaptured; i++) {
			SymFromAddr(Proc, (DWORD64)(Buf[i]), 0, symbol);
			Println("%s - 0x%p\n", symbol->Name, symbol->Address);
		}
		free(symbol);
	}
	static void Logf(const char* fmt, ...) {
		if (!sm.IsDebugEnabled || !sm.ActiveLevels[(int)legit::eLogLevel::INFO]) {
			return;
		}
		va_list list{};
		va_start(list, fmt);
		char Buffer[1028]{};
		vsnprintf_s(Buffer, sizeof(Buffer), fmt, list);
		va_end(list);
		printf("[INFO] %s", Buffer);
	}
	static void Warnf(const char* fmt, ...) {
		if (!sm.IsDebugEnabled || !sm.ActiveLevels[(int)legit::eLogLevel::WARNING]) {
			return;
		}
		va_list list{};
		va_start(list, fmt);
		char Buffer[1028]{};
		vsnprintf_s(Buffer, sizeof(Buffer), fmt, list);
		va_end(list);
		printf("[WARNING] %s", Buffer);
	}
	static void Errorf(const char* fmt, ...) {
		if (!sm.IsDebugEnabled || !sm.ActiveLevels[(int)legit::eLogLevel::ERRORF]) {
			return;
		}
		va_list list{};
		va_start(list, fmt);
		char Buffer[1028]{};
		vsnprintf_s(Buffer, sizeof(Buffer), fmt, list);
		va_end(list);
		printf("[ERROR] %s", Buffer);
	}
	static void Fatalf(const char* fmt, ...) {
		if (!sm.IsDebugEnabled || !sm.ActiveLevels[(int)legit::eLogLevel::FATAL]) {
			return;
		}
		va_list list{};
		va_start(list, fmt);
		char Buffer[1028]{};
		vsnprintf_s(Buffer, sizeof(Buffer), fmt, list);
		va_end(list);
		printf("[FATAL] %s\n", Buffer);
		PrintStackTrace(2);
	}
};
#define Assertf(cond) do {if(!(cond)) { legit::lagDebugger::Fatalf("Assertion Failed! %s@@@%d", __FILE__, __LINE__); __debugbreak(); } } while(false)
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
	static constexpr int SizeOfType = sizeof(Type);
	static constexpr DXGI_FORMAT m_TypeFormat = F;
};
template<typename T, DXGI_FORMAT FORMAT>
struct BaseEvaluator {
	static constexpr size_t Size() {
		return sizeof(T);
	}
	using Type = T;
	static constexpr VertexOfType<Type, FORMAT> VertexInformation;
};
template<eVertexType T> struct VertexEvaluater : public BaseEvaluator<void*, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN> { static_assert(true && "Type is unknown, Unsafely casting to void ptr!"); };
template<> struct VertexEvaluater<eVertexType::VECTOR2> : public BaseEvaluator<DirectX::XMFLOAT2, DXGI_FORMAT_R32G32_FLOAT> {};
template<> struct VertexEvaluater<eVertexType::VECTOR3> : public BaseEvaluator<DirectX::XMFLOAT3, DXGI_FORMAT_R32G32B32_FLOAT> {};
template<> struct VertexEvaluater<eVertexType::VECTOR4> : public BaseEvaluator<DirectX::XMFLOAT4, DXGI_FORMAT_R32G32B32A32_FLOAT> {};

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
static_assert((int)eStaticVertexNames::SIZE_OF == sizeof(VERTEX_NAMES) / sizeof(VERTEX_NAMES[0]) && "Not enough VERTEX_NAME formats");
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
#include <vector>
#include <array>
static void* Allocate(size_t size) {
	printf("Alloc Call: %llu\n", size);
	return malloc(size);
}
#define CaseDefVertexSizeOf(Type) case Type: return VertexEvaluater<Type>::Size()
struct SizeOfFormat {
	static size_t GetSize(eVertexType Type) {
		switch (Type) {
			CaseDefVertexSizeOf(eVertexType::VECTOR3);
			CaseDefVertexSizeOf(eVertexType::VECTOR2);
			CaseDefVertexSizeOf(eVertexType::VECTOR4);
			CaseDefVertexSizeOf(eVertexType::MATRIX4X4);
		case(eVertexType::EVT_MAX):
		default:
			return 0;
		}
	}
};
#undef CaseDefVertexSizeOf
template<typename T> void printType(const T& val) {

}
template<> void printType<DirectX::XMFLOAT2>(const DirectX::XMFLOAT2& val) {
	std::cout << val.x << ", " << val.y << "";
}
template<> void printType<DirectX::XMFLOAT3>(const DirectX::XMFLOAT3& val) {
	std::cout << val.x << ", " << val.y << ", " << val.z << "";
}
template<eVertexType... T>
struct lagVertex {
public:
	struct VTX {
		VTX(typename VertexEvaluater<T>::Type const&... var) : ByteArray({  }) {
			size_t offset = 0;
			(
				(memcpy(
					ByteArray.data() + offset,
					&var,
					sizeof(var)
				),
					offset += sizeof(var)),
				...
				);
		}
		char* GetBytes() {
			return ByteArray.data();
		}
		static constexpr size_t Stride = (VertexEvaluater<T>::Size() + ...);
		using Array = std::array<char, Stride>;
		Array ByteArray;
	};
	VTX Vertex;
	using Arr = std::array<eVertexType, sizeof...(T)>;
	lagVertex() {

	}
	lagVertex(typename VertexEvaluater<T>::Type const&... args) : Vertex(args...){

	}
// I Will Optimize away every 3 second chunk of time until I am so lazy I have the entire computer generate the structure itself. 
#define CaseStatementPrintSet(x) case x: {auto Var = Get<VertexEvaluater<x>::Type>(i); std::cout << #x << "{ "; printType(Var); std::cout << " } "; break;}
	void PrintSet() {
		for (int i = 0; i < sizeof...(T); i++) {
			eVertexType typeE = GetTypeAtIndex(i);
			switch(typeE){
				CaseStatementPrintSet(eVertexType::VECTOR2);
				CaseStatementPrintSet(eVertexType::VECTOR3);
				CaseStatementPrintSet(eVertexType::VECTOR4);
				CaseStatementPrintSet(eVertexType::MATRIX4X4);
			case eVertexType::EVT_MAX:
			default:
				break;
			}
		}
	}
#undef CaseStatementPrintSet 
	template<typename T> T& Get(int Slot) {
		auto* ptr = reinterpret_cast<T*>(GetByteFromPosition(Slot));
		return *ptr;
	}
	eVertexType GetTypeAtIndex(int Identifier) {
		Arr Arguments = { T... };
		return Arguments[Identifier];
	}
	char* GetByteFromPosition(int Slot) {
		Assertf(Slot > sizeof...(T));
		char* Starter = Vertex.GetBytes();
		int AdditionSlot = GetByteOffset(Slot);
		return Starter + AdditionSlot;
	}
	size_t GetByteOffset(int Slot) {
		Arr Arguments = { T... };
		size_t Offset{};
		for (int i = 0; i < Slot; i++) {
			Offset += SizeOfFormat::GetSize(Arguments[i]);
		}
		return Offset;
	}
};

template<typename... V> struct lagStaticVertexFormat {
#ifdef LiveBuild
	static lagInputAssembler* CreateLayout(const lagShaderBytecode& byteCode) {
		constexpr std::array<D3D11_INPUT_ELEMENT_DESC, sizeof...(V)> arr = {
			VertexFormatConversion<V>::GetDescription()...
		};
		lagInputAssembler* m_InputLayout = new lagInputAssembler(arr.data(), arr.size(), byteCode);
		return m_InputLayout;
	}
#endif
	using VertexType = lagVertex<V::Type...>;
	using VectorType = std::vector<VertexType>;
	void AppendVertex(typename VertexEvaluater<V::Type>::Type const&... args) {
		m_Vertices.emplace_back(args...);
	}
	template<typename FuncPtr = void(*)(VertexType&)> void ForeachVertex(FuncPtr m_Function) {
		for (auto& a : m_Vertices) {
			m_Function(a);
		}
	}
	// --YOU SHOULD NEVER GO THROUGH THIS FUNCTION! UNLESS YOU PLAN ON MODIFYING THE DATA IN SOME SIGNIFICANT WAY PLEASE GOD DO NOT DO THIS!!!--
	VectorType& Vector() {
		return m_Vertices;
	}
	const VectorType& Vector() const {
		return m_Vertices;
	}
private:
	VectorType m_Vertices;
};
using Format = lagStaticVertexFormat<
	lagStaticVertexDeclaration<eStaticVertexNames::POSITION, eVertexType::VECTOR3, 0>,
	lagStaticVertexDeclaration<eStaticVertexNames::UVCOORD, eVertexType::VECTOR3, 0>
>;
void VertexPrinter(Format::VertexType& type) {
	type.PrintSet();
}
int main() {
	legit::lagDebugger::Init();

	CLogger::Init();
	CLogger::EnableLoggingLayer(legit::eLogLevel::INFO);
	CLogger::EnableLoggingLayer(legit::eLogLevel::WARNING);
	CLogger::EnableLoggingLayer(legit::eLogLevel::ERRORF);
	CLogger::EnableLoggingLayer(legit::eLogLevel::FATAL);

	Format f;
	f.AppendVertex({ 123, 0, 124 }, { 46,05,05 });
	auto& a = f.Vector()[0].Get<DirectX::XMFLOAT3>(2);
	f.ForeachVertex(VertexPrinter);
	std::cout << "\n";
	a.x = 40;
	f.ForeachVertex(VertexPrinter);

	legit::lagDebugger::Destroy();
	return 0;
}