#pragma once
#include "Size.h"
#include <LITemplates/alloc/Default.h>
#include <LITemplates/debugging/litlogger.h>
#include <LITemplates/debugging/assert.h>
namespace legit {
	template<typename T> class FixedArray {
	public:
		FixedArray() {
			leFatalf(" This array type does not support a \"no-size initialization.\" Please use a different array type provided OR instanciate a size.");
		}
		FixedArray(Size S) {
			pArray = legit::MemoryAllocate<T>(S);
			for (Size i = 0; i < S; i++) {
				new (&pArray[i]) T(); // placement-new lookup to get more detail. might make a macro of it because it looks weird. 
			}
			m_Capacity = S;
			leInfof(" Creating array with size of %ull\n", S);
		}
		FixedArray(Size S, const T& copy) {
			pArray = legit::MemoryAllocate<T>(S);
			for (Size i = 0; i < S; i++) {
				new (&pArray[i]) T(copy); // placement-new lookup to get more detail. might make a macro of it because it looks weird. 
			}
			m_Capacity = S;
			leInfof(" Creating array with size of %ull\n", S);
		}
		void Push(const T& copy) {
			Assertf(m_Size < m_Capacity && " The size of this instance of the array is too large.");
			this->pArray[m_Size] = copy;
			m_Size++;
		}
		void Push(T&& move) {
			Assertf(m_Size < m_Capacity && " The size of this instance of the array is too large.");
			this->pArray[m_Size] = std::move(move);
			m_Size++;
		}
		template<typename... Args> void Emplace(Args&&... args) {
			this->pArray[m_Size] = T(args...);
			m_Size++;
		}
		T& At(Size Pos) {
			Assertf(Pos < m_Size);
			return pArray[Pos];
		}
		const T* Get() const {
			return this->pArray;
		}
		Size GetSize() const {
			return this->m_Size;
		}
		Size GetCapacity() const {
			return this->m_Capacity;
		}
		~FixedArray() {
			for (Size i = 0; i < m_Size; i++) {
				pArray[i].~T();
			}
			legit::Free(pArray);
			leInfof(" Cleared array\n");
		}
	private:
		T* pArray = nullptr;
		Size m_Size{0};
		Size m_Capacity{0};
	};
}