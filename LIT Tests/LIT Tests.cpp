#include <iostream>
#include <LITemplates/func/delegates.h>
#include <LITemplates/pointers/Auto.h>
#include <LITemplates/types/vectortypes.h>
#include <LITemplates/datastructs/dynamicarray.h>
#include <Windows.h>
#include <windowsx.h>
#include <LIT Tests/input/ioInput.h>
void bufPrintf(const char* buff) {
	printf("%s", buff);
}
static void InitLegit() {
	legit::LITLogger::OutputToFunction(bufPrintf);
}
class SampleObject {
public:
	SampleObject() {
		printf("[SAMPLEOBJECT] Creating Object Default Initialization\n");
	}
	SampleObject(int operation) {
		printf("[SAMPLEOBJECT] Creating Object Integer Constructor\n");
		this->Value = operation;
	}
	SampleObject(const SampleObject& copy) {
		printf("[SAMPLEOBJECT] Copying Object using Copy Constructor\n");
		this->Value = copy.Value;
	}
	SampleObject& operator=(const SampleObject& copy) {
		printf("[SAMPLEOBJECT] Copying Object using Copy Equals\n");
		this->Value = copy.Value;
		return *this;
	}
	SampleObject(SampleObject&& move) noexcept {
		printf("[SAMPLEOBJECT] Moving Object using Move Constructor\n");
		this->Value = move.Value;
		move.Value = 0;
	}
	SampleObject& operator=(SampleObject&& move) noexcept {
		printf("[SAMPLEOBJECT] Moving object using Move Equals\n");
		this->Value = move.Value;
		move.Value = 0;
		return *this;
	}
	~SampleObject() {
		printf("[SAMPLEOBJECT] Deleting Object\n");
	}
	bool operator==(const SampleObject& o) {
		if (o.Value == Value) return true;
		return false;
	}
	int Value{};
};


namespace legit {
	template<typename T> class Castable {
	public:
		Castable(T* pointer) {
			this->Pointer = pointer;
		}
		template<typename K>
		K GetAs() {
			return (K)(this->Pointer);
		}
		T* GetPointer() {
			return this->Pointer;
		}
	private:
		T* Pointer;
	};
}
class CSystem {
public:
	static legit::Castable<void> GetInstance() {
		return GetModuleHandle(NULL);
	}
	static legit::Castable<void> GetCurrentKeyboardLayout() {
		return GetKeyboardLayout(0);
	}
private:
};
class ApplicationWindowProcessor {
	struct sWindowInformation {
		bool IsCloseRequested;
		bool IsQuitRequested;
		bool WasWindowJustCreated;
		unsigned long WindowWidth, WindowHeight;
	};
public:
	static constexpr char IGNORE_RESPONSE = 0;
	static void Init() {
		/*
			This would leak because statics aren't uninitialized until after the program closes. (still doesn't explain the 16 bytes of difference I had but whatever. ) 
		*/
		pProc = new std::vector<WNDPROC>{};
	}
	static void AddWindowProc(WNDPROC proc) {
		pProc->emplace_back(proc);
	}
	static void Shutdown() {
		delete pProc;
	}
	static void Update() {
		MSG pMsg{};
		while (PeekMessage(&pMsg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&pMsg);
			DispatchMessageW(&pMsg);
			if (pMsg.message == WM_QUIT) {
				WindowInfo.IsQuitRequested = true;
			}
		}
	}
	static LRESULT CALLBACK MainWindowProc(HWND wnd, UINT m, WPARAM wParam, LPARAM lParam) {
		switch (m) {
			case WM_NCCREATE:
				WindowInfo.WasWindowJustCreated = true;
				break;
			case WM_CREATE:
				{
					CREATESTRUCT* pStruct = (CREATESTRUCT*)lParam;
					WindowInfo.WindowHeight = pStruct->cy;
					WindowInfo.WindowWidth = pStruct->cx;
					WindowInfo.WasWindowJustCreated = false;
					break;
				}
			case WM_CLOSE:
				WindowInfo.IsCloseRequested = true;
				break;
			case WM_SIZE:
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);
				WindowInfo.WindowWidth = width;
				WindowInfo.WindowHeight = height;
				break;
		}
		for (const auto& a : *pProc) {
			LRESULT result = a(wnd, m, wParam, lParam);

			if (result != IGNORE_RESPONSE)
				return result;
		}
		return DefWindowProc(wnd, m, wParam, lParam);
	}
	static sWindowInformation& Get() {
		return WindowInfo;
	}
private:
	static inline sWindowInformation WindowInfo{};
	static inline std::vector<WNDPROC>* pProc;
};
#include <array>
namespace legit {
	namespace EnumUtil {
		template<typename T, typename Enum> static constexpr auto Underly(Enum&& e) {
			return (T)e;
		}
	}
}

namespace legit {
	constexpr unsigned char MAX_BYTE = 0xff;
	constexpr unsigned char MAX_SBYTE = 0x7f;

	constexpr unsigned short MAX_USHORT = 0xFFFF;
	constexpr signed short MAX_SHORT = 0x7FFF;

	constexpr signed long MAX_LONG = 0x7FFFFFFF;
	constexpr unsigned long MAX_ULONG = 0xFFFFFFFF;

	constexpr unsigned long long MAX_ULLONG = 0xFFFFFFFFFFFFFFFF;
	constexpr signed long long MAX_LLONG = 0x7FFFFFFFFFFFFFFF;
}


#include <algorithm>
namespace legit {
	template<typename T> const T& Clamp(const T& Value, const T& Minimum, const T& Maximum) {
		return std::clamp(Value, Minimum, Maximum);
	}
	template<typename T> const T& Abs(const T& Value) {
		return std::abs(Value);
	}
}
class CAppl {
public:
	static void Init() {
		ApplicationWindowProcessor::Init();
		const wchar_t CLASS_NAME[] = L"WindowClass";
		WNDCLASS wc = { };
		wc.lpfnWndProc = ApplicationWindowProcessor::MainWindowProc;
		wc.hInstance = CSystem::GetInstance().GetAs<HINSTANCE>();
		wc.lpszClassName = CLASS_NAME;
		ATOM res = RegisterClassW(&wc);
		if (!res) {
			printf("Window Register Class failed, %d\n", GetLastError());
			__debugbreak();
		}
		Window = CreateWindowEx(
			0,                              // Optional window styles.
			CLASS_NAME,                     // Window class
			L"Learn to Program Windows",    // Window text
			WS_OVERLAPPEDWINDOW,            // Window style

			// Size and position
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

			NULL,       // Parent window    
			NULL,       // Menu
			CSystem::GetInstance().GetAs<HINSTANCE>(),  // Instance handle
			NULL        // Additional application data
		);
		if (Window == NULL) {
			printf("Failed to create window %d\n", GetLastError());
			__debugbreak();
			return;
		}
		ShowWindow((HWND)Window, SW_SHOW);
		legit::ioInput::Init(ApplicationWindowProcessor::AddWindowProc);
	}
	static void Update() {
		while (!ApplicationWindowProcessor::Get().IsCloseRequested) {
			auto i = legit::ioInput::GetGamePad().GetPressureValueNorm(legit::ioGamePadPressureInputs::RT);
			if (i != 0.0000f) {
				float f = i * (float)legit::MAX_USHORT;
				printf("Pressure down %f VibOut: %f\n", i, f);
				legit::ioInput::GetGamePad().Vibrate((short)f);
			} else if (legit::ioInput::GetKeyboard().IsDown(legit::ioKey::A)) {
				legit::ioInput::GetGamePad().Vibrate(legit::MAX_USHORT);
			}
			else {
				legit::ioInput::GetGamePad().Vibrate(0);
			}
			ApplicationWindowProcessor::Update();
			legit::ioInput::Update(ApplicationWindowProcessor::Get().WindowWidth, ApplicationWindowProcessor::Get().WindowHeight);
		}
	}
	static void Shutdown() {
		legit::ioInput::Shutdown();
		CloseWindow((HWND)Window);
		Window = nullptr;
		ApplicationWindowProcessor::Shutdown();
	}
private:
	static inline void* Window = nullptr;
};

class Entry {
public:
	static int Main(int, char**) {
		CAppl::Init();
		CAppl::Update();
		CAppl::Shutdown();
		return 0;
	}
};













#define ENTRY(func) namespace legit{ struct ProgramEntry{ static inline int(*Entry)(int, char**) = func;};}
ENTRY(Entry::Main);
int main(int argc, char** argv) {
	return legit::ProgramEntry::Entry(argc, argv);
}

