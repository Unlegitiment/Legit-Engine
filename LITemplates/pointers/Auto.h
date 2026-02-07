#pragma once
#include <LITemplates\debugging\litlogger.h>
#include <LITemplates\alloc\Default.h>
namespace legit {
	template<typename T> class AutoPtr {
	public:
		AutoPtr(const AutoPtr<T>&) = delete;
		AutoPtr<T>& operator=(const AutoPtr<T>&) = delete;
		AutoPtr(T&& AllocationCall) { // we know we are in a Steal position
			Data = AllocationCall;
			AllocationCall = nullptr;
			leInfof(" AutoPtr now handling 0x%p\n", Data);
		}
		AutoPtr(AutoPtr<T>&& move) {
			this->Data = move.Data;
			move.Data = nullptr;
			leInfof(" Movement of AutoPtr\n");
		}
		T Get() {
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
		T Data{nullptr};
	};
}