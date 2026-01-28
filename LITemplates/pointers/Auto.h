#pragma once
#include <LITemplates\debugging\litlogger.h>
#include <LITemplates\alloc\Default.h>
namespace legit {
	template<typename T> class AutoPtr {
	public:
		AutoPtr(T*& AllocationCall) { // Copy
			Data = legit::New<T>(*AllocationCall);
			leInfof(" AutoPtr is copying value ( object not constructed with either move )\n");
		}
		AutoPtr(const T*& AllocationCall) { // this should be both steal and copy, but for whatever reason it doesn't work
			Data = const_cast<T*>(AllocationCall);
			leInfof(" AutoPtr created via const T*& which is weird.\n");
		}
		AutoPtr(T*&& AllocationCall) { // we know we are in a Steal position
			Data = AllocationCall;
			leInfof(" AutoPtr now handling 0x%p\n", Data);
		}
		AutoPtr(const AutoPtr<T>& copy) {
			Data = legit::New<T>(copy.Data);
			leInfof(" Copy created of AutoPtr\n");
		}
		AutoPtr(AutoPtr<T>&& move) {
			this->Data = move.Data;
			move.Data = nullptr;
			leInfof(" Movement of AutoPtr\n");
		}
		T* Get() {
			return Data;
		}
		template<typename... Args> static constexpr AutoPtr<T> Make(Args&&... args) {
			return legit::New<T>(args...);
		}
		~AutoPtr() {
			leInfof(" Deleting AutoPtr\n");
			legit::Delete(Data);
		}
	private:
		T* Data{nullptr};
	};
}