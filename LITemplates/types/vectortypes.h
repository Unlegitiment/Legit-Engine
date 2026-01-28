#pragma once
#include <intrin.h>
namespace legit {
#define __TypeVectorDefine(x)									\
	using Float##x = __m##x;									\
	using Double##x = __m##x##d;								\
	using Int##x = __m##x##i;									\
	namespace __##x##Priv {										\
		template<typename T>									\
		struct Fetch {											\
			using Type = void;									\
		};														\
		template<> struct Fetch<float> {						\
			using Type = ::legit::Float##x;						\
		};														\
		template<> struct Fetch<double> {						\
			using Type = ::legit::Double##x;					\
		};														\
		template<> struct Fetch<int> {							\
			using Type = ::legit::Int##x;						\
		};														\
	}															\
	template<typename T> using Value##x = typename __##x##Priv::Fetch<T>::Type;	 
	
	__TypeVectorDefine(128);
	__TypeVectorDefine(256);
	__TypeVectorDefine(512);
#undef __TypeVectorDefine
}