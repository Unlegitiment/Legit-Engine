#pragma once
#include <exception>
class CException : public std::exception {
public:
	CException() = default;
	CException(const char* lit) : m_Exception(lit) {

	}
	virtual const char* Reason() const {
		return this->m_Exception;
	}
private:
	const char* what() const throw() {
		return Reason();
	}
	const char* m_Exception = "";
};