#include <stdio.h>
#include <iostream>
#include <DirectXMath.h>
#include <dxgiformat.h>
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
	lagStaticVertexDeclaration<eStaticVertexNames::UVCOORD, eVertexType::VECTOR2, 0>
>;
void VertexPrinter(Format::VertexType& type) {
	type.PrintSet();
}
int main() {
	Format f;
	f.AppendVertex({ 123, 0, 124 }, { 46,05 });
	auto& a = f.Vector()[0].Get<DirectX::XMFLOAT3>(0);
	f.ForeachVertex(VertexPrinter);
	std::cout << "\n";
	a.x = 40;
	f.ForeachVertex(VertexPrinter);
	return 0;
}