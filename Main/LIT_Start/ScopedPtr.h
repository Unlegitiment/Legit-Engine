#pragma once
template<typename T> class ScopedPtr {
public:
	using DeleteFn = void(*)(T*);
	ScopedPtr(T* Scope, DeleteFn CustomDelete) : m_pScope(Scope), m_pFunc(CustomDelete) {

	}
	ScopedPtr(T* Scope) : m_pScope(Scope) {}
	T& operator->() { return this->m_pScope; }
	T& operator*() { return *this->m_pScope; }
	bool operator==(T* OP) {
		return m_pScope == OP;
	}
	T* Get() {
		return this->m_pScope;
	}
	~ScopedPtr() {
		if (m_pFunc) {
			m_pFunc(m_pScope);
		}
		else {
			delete m_pScope;
		}
		m_pScope = nullptr;
	}
private:
	DeleteFn m_pFunc = nullptr;
	T* m_pScope;
};