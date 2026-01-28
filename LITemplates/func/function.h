#pragma once
#include <LITemplates/debugging/litlogger.h>
namespace legit {
	template<typename R> class Function {
	public:
		Function(R&& value) {
			Value = value; 
		}
		Function(const Function<R>& v) {
			this->Value = v.Value;
		}
		Function& operator=(const Function<R>& v) {
			this->Value = v.Value;
			return *this;
		}
		Function(Function&& v) {
			this->Value = v.Value;
			v.Value = nullptr;
		}
		Function& operator=(Function&& v) {
			this->Value = v.Value;
			v.Value = nullptr;
			return *this;
		}
		template<typename... Args> void Invoke(Args&&... args) {
			Value(args...);
		}
		R AsOriginal() {
			return Value;
		}
		~Function() {

		}
	private:
		R Value;
	};
}