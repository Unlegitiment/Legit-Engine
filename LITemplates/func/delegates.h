#pragma once
#include <vector>
#include <LITemplates\debugging\litlogger.h>
namespace legit {
	template<typename R>
	class Delegate {
	public:
		Delegate() = default;
		Delegate(R&& Function) {
			m_Functions.push_back(Function);
		}
		void operator+=(R&& Function) {
			m_Functions.push_back(Function);
			leInfof("[Delegates] Appending new Handler\n");
		}
		template<typename... T> void Invoke(T&&... arguments) {
			leInfof("[Delegates] Invoking functions\n");
			for (const auto& Func : m_Functions) {
				Func((arguments) ...);
			}
		}
	private:
		std::vector<R> m_Functions; // I should really implement the core data structures lmao.
	};
}