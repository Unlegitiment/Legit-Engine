#pragma once
#include <stdio.h>
namespace legit {
	/*
		This functionality is OPT-IN. The user must provide it themself, this allows for debugging to be maintained in the library. 
	*/
	class LITLogger {
	public:
		using Func = void(*)(const char*);
		static void OutputToFile(void* fOutput) {
			sm_pOutputFile = fOutput;
		}
		static void OutputToFunction(Func OutputFunc) {
			sm_pOutputFunc = OutputFunc;
		}
		template<typename... T>
		static void Write(const char* Fmt, T&&... args) {
			if (!IsLoggerActive()) return;
			if (sm_pOutputFile) {
				fprintf_s((FILE*)sm_pOutputFile, Fmt, args...);
			} 
			if (sm_pOutputFunc) {
				char Output[2048] = {0};
				sprintf_s(Output, 2048, Fmt, args...);
				sm_pOutputFunc(Output);
			}
		}
		static bool IsLoggerActive() {
			return sm_pOutputFunc || sm_pOutputFile;
		}
	private:
		static inline void* sm_pOutputFile {nullptr};
		static inline Func sm_pOutputFunc  = nullptr;
	};
}
#define leInfof(fmt, ...) ::legit::LITLogger::Write("[LEGIT_ENGINE][INFO][%s]" fmt, __FUNCTION__, __VA_ARGS__)
#define leWarnf(fmt, ...) ::legit::LITLogger::Write("[LEGIT_ENGINE][WARNING][%s]" fmt, __FUNCTION__, __VA_ARGS__)
#define leErrorf(fmt, ...) ::legit::LITLogger::Write("[LEGIT_ENGINE][ERROR][%s]" fmt, __FUNCTION__, __VA_ARGS__)
#define leFatalf(fmt, ...) ::legit::LITLogger::Write("[LEGIT_ENGINE][FATAL][%s -> %s:::%d]" fmt, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); __debugbreak()