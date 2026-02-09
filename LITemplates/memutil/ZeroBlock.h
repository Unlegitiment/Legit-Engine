#pragma once
#include <LITemplates/types/integrals.h>
namespace legit {
	using Size = legit::u64;
	template<typename T> void ZeroBlock(T* Pointer, legit::Size Count) {
		ZeroMemory(Pointer, sizeof(T) * Count);
	}
	template<typename T> void ZeroBlock(T* Pointer) {
		ZeroBlock(Pointer, 1);
	}
	template<typename T, size_t Size> void ZeroBlock(T(&array)[Size]) {
		ZeroMemory((array[0]), sizeof(T) * Size); // this just calls memset lmao. 
	}
}