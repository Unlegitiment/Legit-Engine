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
		NO_CLASSIFIER,
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
			RNorm,
			RYellow,
			RHIRed,
			RRed,
			RHIPurple,
		};
		static constexpr const char* LevelToName[] = {
			"",
			"INFO",
			"WARN",
			"ERROR",
			"FATAL",
			"TRACE",
		};
	public:
		static constexpr legit::u64 SizeOfBuffer = Size;
		using lagDebugFunctor = void(*)(eLogLevel lvl, const char* msg);
		static void DefaultPrintFuncLag(eLogLevel lvl, const char* msg) {
			if (lvl != eLogLevel::NO_CLASSIFIER) {
				printf("%s[%s] %s\n", LevelToColor[(legit::u8)lvl], LevelToName[(legit::u8)lvl], msg);
			}
			else {
				printf("%s", msg);
			}
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
		void NoClassificationPrintf(const char* fmt, ...) {
			if (!m_bIsDbgLayerEnabled) return;
			va_list list;
			va_start(list, fmt);
			char buffer[Size]{0};
			vsnprintf_s(buffer, sizeof(buffer), fmt, list);
			va_end(list);
			Print(eLogLevel::NO_CLASSIFIER, buffer);
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
		FuncDefLog(NoClassificationPrintf);
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
			Println("%s - 0x%p\n" RNorm, symbol->Name, symbol->Address);
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
	NORMAL,
	SIZE_OF
};
inline constexpr const char* VERTEX_NAMES[] = {
	"POSITION",
	"COLOR",
	"UVCOORD",
	"NORMAL"
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
#include <unordered_map>
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
// So how does one want to do this? Meaning what should the structure look like? 
// So i need to effectively bind VTX addresses to a custom output. We should not be adding VertexStrides on the fly. Meaning that the stride won't change past construction.
// I also need to take what is effectively an initializer list or more accurately a array of objects


#define DECLARE_PRIMITIVES()\
using u8 = unsigned char;\
using u16 = unsigned short;\
using u32 = unsigned long;\
using u64 = unsigned long long;\
using s8 = signed char;\
using s16 = signed short;\
using s32 = signed long;\
using s64 = signed long long

namespace lag {
	DECLARE_PRIMITIVES();
	using lagWideChar = wchar_t;
	using lagByte = u8;
	using lagSignedByte = s8;
	template<typename T> u64 SizeOf() { return sizeof(T); }
	template<typename... T> u64 CollectiveSizeOf() {
		u64 Size{};
		((Size += SizeOf<T>()), ...);
		return Size;
	}
	template<typename T>
	lagByte ToBytes(T data) {
		return *reinterpret_cast<lagByte*>(&data);
	}
	template<typename T>
	lagByte ToBytes(T* data) {
		return *reinterpret_cast<lagByte*>(data);
	}
}
void VertexPrinter(Format::VertexType& type) {
	type.PrintSet();
}
void EnableLog() {
	legit::lagDebugger::Init();
	CLogger::Init();
	CLogger::EnableLoggingLayer(legit::eLogLevel::INFO);
	CLogger::EnableLoggingLayer(legit::eLogLevel::WARNING);
	CLogger::EnableLoggingLayer(legit::eLogLevel::ERRORF);
	CLogger::EnableLoggingLayer(legit::eLogLevel::FATAL);
}
void ShutdownLog() {
	CLogger::Shutdown();
	legit::lagDebugger::Destroy();
}
template<int Size> void PrintAll(lag::lagByte(&b)[Size]) {
	for (int i = 0; i < Size * 8; i++) {
		char b = (b << i) & 0xff;
		printf("%d", b);
	}
}
void PrintAll(lag::lagByte* b, lag::u32 Size) {
	for (int i = 0; i < Size; i++) {
		lag::lagByte& c = b[i];
		printf("0x%x ", c & 0xff);
	}
}
void PrintBits(lag::lagByte* b, lag::u32 Size) {
	for (int i = 0; i < Size; i++) {
		lag::lagByte& c = b[i];
		printf("0x%x ", (c) & 0xff);
	}
}
template<typename T> struct AsPrintString {
	static const char* Return() {
		return "%s";
	}
};
template<> struct AsPrintString<char> {
	static const char* Return() {
		return "%c";
	}
};
template<> struct AsPrintString<float> {
	static const char* Return() {
		return "%f";
	}
};
template<> struct AsPrintString<double> {
	static const char* Return() {
		return "%lf";
	}
};
template<> struct AsPrintString<int> {
	static const char* Return() {
		return "%d";
	}
};
template<> struct AsPrintString<unsigned long long> {
	static const char* Return() {
		return "%llu";
	}
};
template<> struct AsPrintString<DirectX::XMFLOAT3> {
	static const char* Return() {
		return "%f %f %f";
	}
};
/*
	Purpose: Printing byte buffers of variable size to the console As another type via automatic conversion. 
	Dependancies: lagDebugger, AsPrintString<T>::Return();
	Note: This should be used with primitives or items that convert to primitives well, (types like, char, float, int etc.) 
		If this is used with a non-primitive type it likely will not work because the bytes will be improperly converted.
*/
template<typename T>
void PrintBytesAs(lag::lagByte* b, lag::u32 Size) {
	for (int i = 0; i < Size / sizeof(T); i++) {
		auto a = i * sizeof(T);
		lag::lagByte* c = b + (a);
		T f = *(T*)c;
		legit::lagDebugger::Println(AsPrintString<T>::Return(), f);
	}
}
/*
	Purpose: Repeated version of PrintBytesAs<>(lag::lagByte*, lag::u32), allows for multiple byte interpretations debugged to the console.
	Dependancies: lagDebugger, PrintBytesAs<T>(lag::lagByte*, lag::u32);
*/
template<typename... T> void PrintBytesAsTypes(lag::lagByte* b, lag::u32 Size) {
	(
		(
			legit::lagDebugger::Println("Type: %s", typeid(T).name()),
			PrintBytesAs<T>(b,Size),
			legit::lagDebugger::NoClassificationPrintf("\n")
			),
		...
	);
	return;
}


namespace lit {
	DECLARE_PRIMITIVES();
	
	template<bool T>
	struct Type {
		static constexpr bool Evaluation = T;
	};
	struct TrueT : public Type<true> {};
	struct FalseT : public Type<false>{};

	template<bool... T> struct IsAllTrue : public FalseT {};
	template<> struct IsAllTrue<true> : public TrueT {};

	template<typename Arg1, typename Arg2> struct IsSameT : public FalseT {};
	template<typename Arg1> struct IsSameT<Arg1, Arg1> : public TrueT {};

	template<typename Arg> struct IsRValue : public FalseT {};
	template<typename Arg> struct IsRValue<Arg&&> : public TrueT{};


	template<typename T, bool B> struct EnableIf : public FalseT{
		using Type = void;
	};
	template<typename T> struct EnableIf<T, true> : public TrueT {
		using Type = T;
	};
	template<typename T> struct RemoveReference {
		using Type = T;
	};
	template<typename T> struct RemoveReference<T&> {
		using Type = T;
	};
	template<typename T> struct RemoveReference<T&&> {
		using Type = T;
	};
	template<typename T> struct AddRef {
		using Type = T&;
	};
	template<typename T> struct AddRef<T&> {
		using Type = T;
	};
	template<typename T> struct AddRef<T&&> {
		using Type = T&;
	};
	template<typename T> typename AddRef<T>::Type DeclaredValue();
	template<typename T>
	struct IsCopyable {
	private:
		template<typename U, typename = decltype(U(DeclaredValue<const U&>()))> static TrueT Test(int);
		template<typename> static FalseT Test(...);
	public:
		static constexpr bool Value = decltype(Test<T>(1))::Evaluation;
	};



#ifdef _MSC_VER
	struct IsMSVCEnabled : public TrueT{
#else 
	struct IsMSVCEnabled : FalseT{
#endif
	};
	template<typename T, typename R> T StaticCast(R&& val) {
		return static_cast<T>(val);
	}
	template<typename T> typename RemoveReference<T>::Type&& Move(T&& arg) {
		return StaticCast<typename RemoveReference<T>::Type&&>(arg);
	}

	template<typename T> T* MemoryMove(T* Destination, const T* Source, u32 Amount) {
		Assertf(Destination);
		Assertf(Amount > 0);
#ifdef LIT_MMVE_CUSTOM
		for (int i = 0; i < Amount; i++) {
			Destination[i] = Source[i];
		}
		return Destination;
#else
		return (T*)memmove(Destination, Source, sizeof(T) * Amount);
#endif
	}
	template<typename T> T* MemoryCopy(T* Destination, const T* Source, u32 Amount) noexcept {
		Assertf(Destination);
		Assertf(Amount > 0);
		return (T*)memcpy(Destination, Source, sizeof(T) * Amount);
	}
	template<typename T> T* Copy(T* Destination, const T* Source, u32 Amount) noexcept {
		if constexpr (IsCopyable<T>::Value) {
			MemoryCopy(Destination, Source, Amount);
			return Destination + Amount;
		}
		for (u32 i = 0; i < Amount; i++) {
			Destination[i] = Source[i];
		}
		return Destination + Amount;
	}
	template<typename T> class DynamicArray {
	public:
		DynamicArray() {
			
		}
		DynamicArray(DynamicArray<T>&& other) { // move
			this->Buffer = other.Buffer;
			this->Capacity = other.Capacity;
			this->Size = other.Size;
			other.Buffer = nullptr;
			other.Capacity = 0;
			other.Size = 0;
		}
		DynamicArray<T>& operator=(DynamicArray<T>&& other) {
			this->Buffer = other.Buffer;
			this->Capacity = other.Capacity;
			this->Size = other.Size;
			other.Buffer = nullptr;
			other.Capacity = 0;
			other.Size = 0;
			return *this;
		}
		DynamicArray(const DynamicArray<T>& other) {
			
		}
		~DynamicArray() {
			delete Buffer;
		}
		void Allocate(u32 Size) {
			this->Size = 0;
			this->Capacity = Size * 2;
			if (this->Buffer) {
				delete Buffer;
			}
			Buffer = new T[Capacity];
		}
		T* GetData() {
			return this->Buffer;
		}
		void Push(const T& value) {
			Assertf(Size < Capacity);
			Assertf(Buffer);
			Buffer[Size] = value;
			Size++;
		}
		void PushAndGrow(const T& value) {
			Buffer[Size] = value;
			Size++;
			Grow(Size * 2); 
		}
		void Grow(u32 newCapacity) {
			T* temp = new T[newCapacity];
			memmove(temp, Buffer, Capacity);
			delete Buffer;
			Buffer = temp;
			Capacity = newCapacity;
			return;
		}
		void Remove(u32 Slot) {
			Assertf(Slot < Size);
			Assertf(Slot < -1);
			//If its greater than the selected slot we want to move it down by one? But how?
			for (int i = 0; i < Size; i++) {
				if (i > Slot) {
					// if the current iterator is greater than the slot shift down by one? 
					MemoryMove(&Buffer[i - 1],&Buffer[i], 1);
				}
			}
			Size--;
		}
		void Clear() {
			delete[] Buffer;
			Buffer = nullptr;
			Size = 0;
			Capacity = 0;
		}
		T& GetAt(u32 Index) {
			Assertf(Index < Size);
			Assertf(Index >= 0);
			Assertf(this->Buffer);
			return Buffer[Index];
		}
		u32 GetSize() const {
			return this->Size;
		}
		u32 GetCapacity() {
			return this->Capacity;
		}
	private:
		T* Buffer = nullptr;
		u32 Size = 0;
		u32 Capacity = 0;
	};
	template<typename T> void DbgPrint(lit::DynamicArray<T>& pV) {
		std::cout << "List Debug: \n\tSize: " << pV.GetSize() << " \n\tCapacity: " << pV.GetCapacity() << "\n";
		for (int i = 0; i < pV.GetSize(); i++) {
			T& a = pV.GetAt(i);
			//std::cout << a << std::endl;
		}
	}
	template<typename T, u64 _Size>
	class StaticArray {
	public:
		constexpr StaticArray() = default;
		~StaticArray() {}
		constexpr u64 Capacity() {
			return _Size;
		}
		constexpr u64 Size() {
			return _Size;
		}
		constexpr void Insert(const T& data, u64 Position) {
			Assertf(_Size > Position && "Cannot perform this.");
			Buffer[Position] = data;
		}
		DynamicArray<T> ToDynamic() {
			DynamicArray<T> Ret;
			Ret.Allocate(_Size);
			Copy(Ret.GetData(), Buffer, _Size);
			return Ret;
		}
	private:
		T Buffer[_Size];
	};
	template<typename T>
	class ScopedPtr {
	public:
		ScopedPtr() = default;
		ScopedPtr(std::decay_t<T>&& rPointer) : Pointer(new T(rPointer)) {

		}
		ScopedPtr& operator=(T*&& rPointer) {
			this->Pointer = rPointer;
			return *(this);
		}
		ScopedPtr(const ScopedPtr<T>&) = delete;
		ScopedPtr& operator=(const ScopedPtr<T>&) = delete;
		ScopedPtr(ScopedPtr<T>&& rhs) {
			this->Pointer = rhs.Pointer;
			rhs.Pointer = nullptr;
		}
		ScopedPtr& operator=(ScopedPtr<T>&& rhs) {
			this->Pointer = rhs.Pointer;
			rhs.Pointer = nullptr;
			return (*this);
		}
		T& operator*() {
			return *this->Pointer;
		}
		T& operator->() {
			return *this->Pointer;
		}
		T* operator&() {
			return this->Pointer;
		}
		ScopedPtr<T>* GetAddressOfScope() {
			return this;
		}
		~ScopedPtr() {
			delete Pointer;
		}
	private:
		T* Pointer = nullptr;
	};
}
class CDynamicTest {
public:
	static void Init() {
		 
		const char* BufferNames[]{
			"POSITION",
			"UVCOORDINATE",
			"NORMAL"
		};
		const int SizeAtEach[]{
			lag::SizeOf<DirectX::XMFLOAT3>(),
			lag::SizeOf<DirectX::XMFLOAT2>(),
			lag::SizeOf<DirectX::XMFLOAT3>() 
		};
		lag::u64 Size = lag::CollectiveSizeOf<DirectX::XMFLOAT3, DirectX::XMFLOAT2, DirectX::XMFLOAT3>();
		lit::ScopedPtr<lag::lagByte*> Buffer{new lag::lagByte[Size]{0}};
		lag::s32 Access = 2; // I want to access THIS item.

		lag::u64 ByteOffset = 0;
		for (int i = 0; i < Access; i++) {
			ByteOffset += SizeAtEach[i];
			Assertf(ByteOffset < Size);
		}
		
		DirectX::XMFLOAT3* pOffset = (DirectX::XMFLOAT3*)(*Buffer + ByteOffset);
		pOffset->x = 4;
		pOffset->y = 4;
		pOffset->z = 4;

		PrintBytesAsTypes<float, int>(*Buffer, Size);
		
	}
	static void Destroy(){
	
	}
private:
};
int MyMain() {
	CDynamicTest::Init();


	CDynamicTest::Destroy();

	return 0;
}
int main() {
	EnableLog();


	MyMain();


	ShutdownLog();
	return 0;
}