#pragma once
#include <LITemplates/debugging/assert.h>
#include <LITemplates/alloc/Default.h>
#include "ContainerAllocator.h"
#include "Size.h"
#include <LITemplates/func/function.h>
namespace legit {
	template<typename T> class DynamicArray {
	private:
		static constexpr int GrowMultiplier = 3;
		ContainerAllocator<T> Allocator{};
	public:
		DynamicArray() {
			leInfof(" Array instanciated with no constructor arguments. This means default size of array will be 0.\n");
		}
		DynamicArray(Size NewCapacity) {
			leInfof(" Array instanciated with Capacity %d\n", NewCapacity);
			this->m_Capacity = NewCapacity; // probably not bueno but its a small copy. 
			Setup();
		}
		DynamicArray(const DynamicArray<T>& Copy) {
			this->m_pArray = Allocator.Allocate(Copy.m_Capacity);
			this->m_Size = Copy.m_Size;
			this->m_Capacity = Copy.m_Capacity;
			for (int i = 0; i < m_Size; i++) {
				Allocator.Instanciate(this->m_pArray[i], Copy.m_pArray[i]);
			}
			leInfof(" Copy constructor called.\n");

		}
		DynamicArray(DynamicArray<T>&& Move) noexcept : m_pArray(Move.m_pArray), m_Size(Move.m_Size), m_Capacity(Move.m_Capacity) {
			Move.m_pArray = nullptr;
			Move.m_Size = 0;
			Move.m_Capacity = 0;
			leInfof(" Movement constructor called.\n");

		}
		void PushAndGrow(const T& copy) {
			if (m_Size + 1 > m_Capacity) {
				Grow();
				leInfof(" PushAndGrow has caused DynamicArray to grow.\n");
			}
			Push(copy);
		}
		void PushAndGrow(T&& move) {
			if (m_Size + 1 > m_Capacity) {
				Grow();
				leInfof(" PushAndGrow has caused DynamicArray to grow.\n");

			}
			Push(std::move(move));
		}
		void Push(const T& copy) {
			Assertf(m_Size < m_Capacity && "Size Greater than Capacity");
			Allocator.Instanciate(m_pArray[m_Size], copy);
			++m_Size;
		}
		void Push(T&& move) {
			Assertf(m_Size < m_Capacity && "Size Greater than Capacity");
			Allocator.Instanciate(m_pArray[m_Size], std::move(move));
			++m_Size;
		}
		template<typename... Args>
		void Emplace(Args&&... args) {
			Assertf(m_Size < m_Capacity && "Size Greater than Capacity");
			Allocator.Instanciate(m_pArray[m_Size], std::forward<Args>(args)...);
			++m_Size;
			leInfof(" used.\n");
		}
		template<typename... Args>
		void EmplaceAndGrow(Args&&... args) {
			if (m_Size + 1 > m_Capacity) {
				Grow();
				leInfof(" Emplace has caused DynamicArray to grow.\n");

			}
			Emplace(args...); // probably should perfectly forward but whatever.
		}
		void Clear() {
			for (Size i = 0; i < m_Size; i++) {
				Allocator.Deconstruct(m_pArray[i]);
			}
			m_Size = 0;
		}
		~DynamicArray() {
			for(Size i = 0; i < m_Size; i++){
				Allocator.Deconstruct(m_pArray[i]);
			}
			Allocator.Deallocate(m_pArray);
			leInfof(" Array Reset\n");
		}
	public:
		const T* GetArray() const {
			return this->m_pArray;
		}
		T* GetArray() {
			return this->m_pArray;
		}
		Size GetSize() const {
			return this->m_Size;
		}
		Size GetCapacity() const {
			return this->m_Capacity;
		}
	public: // Extentions.
		void ForEach(legit::Function<void(*)(T&)> functor) {
			for (int i = 0; i < this->m_Size; i++) {
				functor.Invoke(m_pArray[i]);
			}
		}
		T* Find(const T& comparitor) {
			for (int i = 0; i < this->m_Size; i++) {
				if (m_pArray[i] == comparitor) {
					leInfof("Found Value!\n");

					return &m_pArray[i];
				}
			}
			leInfof("Could not find value!\n");
			return nullptr;
		}
		T* Find(legit::Function<bool(*)(T&)> Evaluate) {
			for (int i = 0; i < m_Size; i++) {
				if (Evaluate.AsOriginal()(m_pArray[i])) {
					leInfof("Found value.\n");
					return &m_pArray[i];
				}
			}
			leInfof("Could not find value!\n");
			return nullptr;
		}
	private:
		void Setup() {
			if (m_Capacity > 0)
				m_pArray = Allocator.Allocate(m_Capacity);
		}
		void Grow() {
			Size newcap = m_Capacity ? m_Capacity * GrowMultiplier : 1;
			T* temp = Allocator.Allocate(newcap); // cannot do this!
			for (int i = 0; i < m_Size; i++) {
				Allocator.Instanciate(temp[i], std::move(m_pArray[i])); // so steal those resources bby. 
				Allocator.Deconstruct(m_pArray[i]);
			}
			m_Capacity = newcap;
			Allocator.Deallocate(m_pArray);
			m_pArray = temp;
			leInfof(" Array grown to Capacity %llu\n", m_Capacity);
		}
		T* m_pArray = nullptr;
		Size m_Size = 0;
		Size m_Capacity = 0;
	};
}