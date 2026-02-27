#pragma once
#include <LITemplates\debugging\litlogger.h>
#include <LITemplates\alloc\Default.h>
#include <memory>
namespace legit {
	template<typename T> using OwnerPtr = std::unique_ptr<T>;
	template<typename T, typename... TArgs> OwnerPtr<T> CreateOwnerPtr(TArgs... args) {
		return std::make_unique<T>(args...);
	}
	template<typename T> using AutoPtr = std::shared_ptr<T>;
	template<typename T, typename... TArgs> AutoPtr<T> CreateAutoPtr(TArgs... args) {
		return std::make_shared<T>(args...);
	}
};