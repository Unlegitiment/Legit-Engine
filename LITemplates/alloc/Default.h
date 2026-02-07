#pragma once
#include <LITemplates/debugging/litlogger.h>
#include <typeinfo>
namespace legit {
	// PURPOSE - Allocates a buffer of size T on the heap. (DOES NOT CALL C_TOR)
	template<typename T> constexpr T* MemoryAllocate() {
		leInfof(" Malloc Called on Type of Size: %llu\n", sizeof(T));
		return (T*)malloc(sizeof(T));
	}
	// PURPOSE - Memory Allocate, Does the same thing as the base, however allows specification of an amount. 
	template<typename T> constexpr T* MemoryAllocate(unsigned long Amount) {
		leInfof(" Function called with Amount %lu Totalling Bytes Allocated: %d\n", Amount, sizeof(T) * Amount);
		return (T*)malloc(sizeof(T) * Amount);
	}
	template<typename T> constexpr T* New() {
		leInfof(" Creating a new: %s\n", typeid(T).name());
		return new T();
	}
	template<typename T, typename... ConstructorArgs> constexpr T* New(ConstructorArgs&&... args) {
		leInfof(" Creating a new: %s\n", typeid(T).name());
		return new T(std::forward<ConstructorArgs>(args)...);
	}
	// PURPOSE - Deletes a dynamically allocated pointer's memory and sets the pointer to nullptr.
	template<typename T> constexpr void Delete(T*& pMem) {
		leInfof(" Deleting a %s at 0x%p\n", typeid(T).name(), pMem);
		delete pMem;
		pMem = nullptr;
	}
	template<typename T> constexpr void Free(T*& pMem) {
		leInfof(" Freeing bytes of size %llu\n", sizeof(T));
		free(pMem);
		pMem = nullptr;
	}
}