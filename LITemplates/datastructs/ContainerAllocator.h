#pragma once
#include <utility>
namespace legit {
	template<class T> class ContainerAllocator {
	public:
		constexpr T* Allocate(unsigned long long Amount) {
			return (T*)::operator new(sizeof(T) * Amount);
		}
		constexpr void Instanciate(T& data, T&& move) {
			new (&data) T(std::move(move));
		}
		constexpr void Instanciate(T& data, const T& copy) {
			new (&data) T(copy);
		}
		template<typename... Args>
		constexpr void Instanciate(T& data, Args&&... Forward) {
			new (&data) T(std::forward<Args>(Forward)...);
		}
		constexpr void Deconstruct(T& data) {
			data.~T();
		}
		constexpr void Deallocate(T*& allocedblock) {
			::operator delete((void*)allocedblock);
			allocedblock = nullptr;
		}
	private:

	};
}