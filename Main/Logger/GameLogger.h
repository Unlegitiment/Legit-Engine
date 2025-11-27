#pragma once
#include <LECore/headers/platform_specs.h>
#ifdef LE_WIN
#include <Windows.h>
#endif // LE_WIN

#include <stdio.h>
#include <varargs.h>
#include <Main/Exception/exception.h>
enum eLogFile {
	ELF_OUT,
	ELF_ERR,
	ELF_IN,
	ELF_MAX
};
extern const char* const g_LogFileNames[ELF_MAX];

class CLogFile {
public:
	CLogFile(eLogFile ELF) : m_FileDesc(ELF){
		freopen_s(&m_FilePtr, g_LogFileNames[ELF], GetModeForFileInput(ELF), GetOldHandle(ELF));
	}
	bool HasWritePermission() {
		return m_FileDesc != ELF_IN;
	}
	static const char* GetModeForFileInput(eLogFile eFile) {
		switch (eFile) {
		case ELF_IN:
			return "r";
		default:
			return "w+";
		}
	}
	static FILE* GetOldHandle(eLogFile f) throw() {
		switch (f) {
		case ELF_OUT: 
			return stdout;
		case ELF_IN:
			return stdin;
		case ELF_ERR:
			return stderr;
		default:
			throw CException("FileOperation not supported."); 
	 	}
	}
	~CLogFile() {
		fclose(m_FilePtr);
		m_FilePtr = nullptr;
	}
	eLogFile GetFileDescription() const {
		return this->m_FileDesc;
	}
private:
	FILE* m_FilePtr = nullptr;
	eLogFile m_FileDesc;
};
class CLogger {
public:

	static void Init() {
#ifdef LE_WIN32
		SetupConsole();
#endif // LE_WIN32
	}

	static constexpr int MAX_LOG_LEN = 1028l;

	static void Log(const char* fmt, ...) {

		va_list list{};
		char buffer[MAX_LOG_LEN] = { 0 };
		va_start(list, fmt);
		{
			_vsnprintf_s(buffer, sizeof(buffer), fmt, list);
		}
		va_end(list);
		printf(buffer); // Don't you worry its gonna be null terminated. 
	}
	static void Shutdown() {
		for (long i = 0; i < ELF_MAX; i++) {
			delete sm_File[i];
		}
		ShutdownConsole();
	}
private:
	CLogger();
#ifdef LE_WIN
	struct sWin32Pipe{
		sWin32Pipe(const CLogFile& lFile) {
			Init(lFile);
		}
		~sWin32Pipe() {
			Shutdown();
		}
		HANDLE GetWrite() const { return m_hWrite; }
		HANDLE GetRead() const { return m_hRead; }
		const SECURITY_ATTRIBUTES& GetSecurityAttributes() const { return m_SecAddr; } // Manages likely a dynamic resource of some sort judging by the lpSecurityDescriptor. (Although this is NULL in our case) 
	private:
		void Init(const CLogFile& lFile) {
			m_SecAddr = { sizeof(m_SecAddr),NULL,TRUE }; // Sec Attr? (This must be a Win32 Thing)
			CreatePipe(&m_hRead, &m_hWrite, &m_SecAddr, 0);
			SetStdHandle(GetSTDHandle(lFile), m_hWrite);
		}
		void Shutdown() {
			CloseHandle(m_hWrite);
			CloseHandle(m_hRead);
			return;
		}
		DWORD GetSTDHandle(const CLogFile& lFile) {
			switch (lFile.GetFileDescription()) {
			case ELF_IN:
				return STD_INPUT_HANDLE;
			case ELF_OUT:
				return STD_OUTPUT_HANDLE;
			case ELF_ERR:
			default:
				return STD_ERROR_HANDLE;
			}
		}
		HANDLE m_hRead{};
		HANDLE m_hWrite{};
		SECURITY_ATTRIBUTES m_SecAddr;
	};
#endif
	static void SetupConsole() {
#ifdef LE_WIN
		if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
			AllocConsole();
		}
		for (long i = 0; i < ELF_MAX; i++) {
			sm_File[i] = new CLogFile((eLogFile)i);
			sm_Pipes[i] = new sWin32Pipe(*sm_File[i]);
		}
		wprintf(L"\n");
		fflush(stdout);
#endif // LE_WIN
	}
	static void ShutdownConsole() {
#ifdef LE_WIN
		for (long i = 0; i < ELF_MAX; i++) {
			delete sm_Pipes[i];
		}

#endif // LE_WIN
	}
	static inline CLogFile* sm_File[ELF_MAX]{};
#ifdef LE_WIN
	static inline sWin32Pipe* sm_Pipes[ELF_MAX]{};
#endif // LE_WIN
};