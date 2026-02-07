#include <iostream>
#include <LITemplates/func/delegates.h>
#include <LITemplates/pointers/Auto.h>
#include <LITemplates/types/vectortypes.h>
#include <LITemplates/datastructs/dynamicarray.h>
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
#include <Windows.h>
#include <windowsx.h>
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
	static void AddWindowProc(WNDPROC proc) {
		pProc.emplace_back(proc);
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
		for (const auto& a : pProc) {
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
	static inline std::vector<WNDPROC> pProc;
};
namespace legit {
	struct ioKeyState {
		bool bIsKeyDown = false;
		bool bIsKeyUp = true;
	};
	enum class ioKey {
		Unknown = 0,
		Escape, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, PrintScreen, ScrollLock, Pause, F13, F14, F15, F16,
		Grave, Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9, Key0, Minus, EqualsPlus, Backspace, Insert, Home, PageUp, NumPadLock, NumPadDivide, NumPadMultiply, NumPadMinus,
		Tab, Q, W, E, R, T, Y, U, I, O, P, OpenBracket, CloseBracket, Backslash, Delete, End, PageDown, NumPad7, NumPad8, NumPad9, NumPadAdd,
		CapsLock, A, S, D, F, G, H, J, K, L, SemiColon, Apostrophe, Enter, NumPad4, NumPad5, NumPad6,
		LShift, Z, X, C, V, B, N, M, Comma, Period, Slash, RShift, Up, NumPad1, NumPad2, NumPad3, NumPadEnter,
		LControl, LWin, LAlt, Space, RAlt, RWin, Menu, RControl, Left, Down, Right, NumPad0, NumPadPeriod,
		MAX_IO_KEY
	};
	class ioKeyboard {
	public:
		friend class ioHandler;
		static constexpr unsigned char MaxKeys = 255;
		ioKeyboard() {}
		void Update() {
			if (!GetKeyboardState(KeyboardState)) {
				printf("LastError[KEYBOARD::UPDATE]: %d\n", GetLastError());
			}
		}
		const BYTE* GetState() const {
			return this->KeyboardState;
		}
		ioKeyState GetState(legit::ioKey key) {
			return KeyState[(int)key];
		}
		bool IsDown(legit::ioKey key) const {
			return KeyState[(int)key].bIsKeyDown;
		}
		bool IsUp(legit::ioKey key) const {
			return KeyState[(int)key].bIsKeyUp; // really fucking useless, lmao. 
		}
	private:
		BYTE KeyboardState[MaxKeys + 1]{0x0}; // specifically for the GetKeyboardState();
		ioKeyState KeyState[int(ioKey::MAX_IO_KEY)]{};
	};
}
#include <array>
namespace legit {
	enum class ioGamePadButtons {
		A, B, X, Y, LB, RB, RJoy, LJoy, Start, Back, DPadUp, DPadDown, DPadRight, DPadLeft,
		MAX_GAMEPAD_BUTTONS
	};
	enum class ioGamePadPressureInputs {
		LT, RT, LeftStickX, RightStickX, LeftStickY, RightStickY,
		MAX_PRESSURES
	};
	class ioGamePad {
	public:
		friend class ioHandler;
		using ButtonValue = bool;
		using PressureValue = short;
		using PressureValueNormalized = float;
		using RumbleValue = unsigned short;
		ButtonValue GetButtonState(ioGamePadButtons button) const {
			return ButtonsState[(int)button];
		}
		PressureValue GetPressureValue(ioGamePadPressureInputs Input) {
			switch (Input) {
				case ioGamePadPressureInputs::LT: return LT;
				case ioGamePadPressureInputs::RT: return RT;
				case ioGamePadPressureInputs::LeftStickX: return LeftStickX;
				case ioGamePadPressureInputs::LeftStickY: return LeftStickY;
				case ioGamePadPressureInputs::RightStickY: return RightStickY;
				case ioGamePadPressureInputs::RightStickX: return RightStickX;
			}
		}
		PressureValueNormalized GetPressureValueNorm(ioGamePadPressureInputs PressureInputs) {
			switch (PressureInputs) {
				case ioGamePadPressureInputs::LT: return (PressureValueNormalized)LT / GetMaximumForGamePadTrigger();
				case ioGamePadPressureInputs::RT: return (PressureValueNormalized)RT / GetMaximumForGamePadTrigger();
				case ioGamePadPressureInputs::LeftStickX: return (PressureValueNormalized)LeftStickX / GetMaximumForGamePadStick();
				case ioGamePadPressureInputs::LeftStickY: return (PressureValueNormalized)LeftStickY / GetMaximumForGamePadStick();
				case ioGamePadPressureInputs::RightStickY: return (PressureValueNormalized)RightStickY / GetMaximumForGamePadStick();
				case ioGamePadPressureInputs::RightStickX: return (PressureValueNormalized)RightStickX / GetMaximumForGamePadStick();
			}
		}
		void Vibrate(RumbleValue MotorSpeed) {
			this->LeftMotor = MotorSpeed;
			this->RightMotor = MotorSpeed;
		}
		/*
		This might not work because the RumbleValue is broken per-motor via XINPUT. Not too sure.
		*/
		void Vibrate(RumbleValue RightMotor, RumbleValue LeftMotor) {
			this->LeftMotor = LeftMotor;
			this->RightMotor = RightMotor;
		}
		bool IsVibrating() const {
			return this->LeftMotor != 0 && this->RightMotor != 0;
		}
	private:
		PressureValueNormalized GetMaximumForGamePadTrigger() {
			return 255.f;
		}
		PressureValueNormalized GetMaximumForGamePadStick() {
			return 32767.0f;
		}
		ButtonValue ButtonsState[(int)ioGamePadButtons::MAX_GAMEPAD_BUTTONS];
		PressureValue LT, RT;
		PressureValue RightStickX, RightStickY;
		PressureValue LeftStickX, LeftStickY;
		RumbleValue RightMotor = 0, LeftMotor = 0;
		
	};
}
namespace legit {
	class ioMouse {
	public:
		friend class ioHandler;
		using IOMouseType = int;
		using DeltaType = int;
		using MouseNormalized = float;
		DeltaType GetDeltaX() const {
			return this->MouseXDelta;
		}
		DeltaType GetDeltaY() const {
			return this->MouseYDelta;
		}
		IOMouseType GetX() const {
			return this->MouseX;
		}
		IOMouseType GetY() const {
			return this->MouseY;
		}
		MouseNormalized GetNormX() const {
			return MouseNormalizedX;
		}
		MouseNormalized GetNormY() const {
			return MouseNormalizedY;
		}
	private:
		DeltaType MouseXDelta;
		DeltaType MouseYDelta;
		IOMouseType MouseX;
		IOMouseType MouseY;
		MouseNormalized MouseNormalizedX;
		MouseNormalized MouseNormalizedY;
	};

	class ioHandler {
	public:
		static void SetMousePrivate(ioMouse* pMouse, ioMouse::DeltaType DeltaX, ioMouse::DeltaType DeltaY, ioMouse::IOMouseType MouseX, ioMouse::IOMouseType MouseY) {
			SetMousePrivate(pMouse, DeltaX, DeltaY, MouseX, MouseY, 0, 0);
		}
		static void SetMousePrivate(ioMouse* pMouse, ioMouse::DeltaType DeltaX, ioMouse::DeltaType DeltaY, ioMouse::IOMouseType MouseX, ioMouse::IOMouseType MouseY, ioMouse::MouseNormalized NormalizedX, ioMouse::MouseNormalized NormalizedY) {
			pMouse->MouseX = MouseX;
			pMouse->MouseY = MouseY;
			pMouse->MouseXDelta = DeltaX;
			pMouse->MouseYDelta = DeltaY;
			pMouse->MouseNormalizedX = NormalizedX;
			pMouse->MouseNormalizedY = NormalizedY;
		}
		static void SetKey(ioKeyboard* pBoard, ioKey key, ioKeyState keyState) {
			pBoard->KeyState[(int)key] = keyState;
		}
		static ioKeyState* GetKeyStateFromBoard(ioKeyboard* pBoard) {
			return pBoard->KeyState;
		}
		static ioMouse::DeltaType* GetMouseXDelta(ioMouse* pMouse) {
			return &pMouse->MouseXDelta;
		}
		static ioMouse::DeltaType* GetMouseYDelta(ioMouse* pMouse) {
			return &pMouse->MouseYDelta;
		}
		/*
			Returns pointer to array containing legit::ioGamePadButtons::MAX_GAMEPAD_BUTTONS size.
		*/
		static legit::ioGamePad::ButtonValue* GetControllerValues(legit::ioGamePad* pGamePad) {
			return pGamePad->ButtonsState;
		}
		static legit::ioGamePad::PressureValue* GetControllerPressure(legit::ioGamePad* pGamePad, legit::ioGamePadPressureInputs SelectedPressure) {
			switch (SelectedPressure) {
				case legit::ioGamePadPressureInputs::LeftStickX: return &pGamePad->LeftStickX;
				case legit::ioGamePadPressureInputs::LeftStickY: return &pGamePad->LeftStickY;
				case legit::ioGamePadPressureInputs::RightStickX: return &pGamePad->RightStickX;
				case legit::ioGamePadPressureInputs::RightStickY: return &pGamePad->RightStickY;
				case legit::ioGamePadPressureInputs::LT: return &pGamePad->LT;
				case legit::ioGamePadPressureInputs::RT: return &pGamePad->RT;
				default: return nullptr;
			}
		}
		static legit::ioGamePad::RumbleValue* GetControllerRumbleRightMotor(legit::ioGamePad* pPad) {
			return &pPad->RightMotor;
		}
		static legit::ioGamePad::RumbleValue* GetControllerRumbleLeftMotor(legit::ioGamePad* pPad) {
			return &pPad->LeftMotor;
		}

		static inline legit::ioGamePad::RumbleValue LastReportedMotorLValue = 0;
		static inline legit::ioGamePad::RumbleValue LastReportedMotorRValue = 0;
	private:
		
	};
}
namespace legit {
	namespace EnumUtil {
		template<typename T, typename Enum> static constexpr auto Underly(Enum&& e) {
			return (T)e;
		}
	}
}
class CIOBasis {
public:
	static void Init() {
		ApplicationWindowProcessor::AddWindowProc(WindowProc);
	}
	static LRESULT WindowProc(HWND wnd, UINT m, WPARAM wParam, LPARAM lParam) {
		switch (m) {
			case WM_MOUSEMOVE:
				MouseX = GET_X_LPARAM(lParam);
				MouseY = GET_Y_LPARAM(lParam);
				break;
		}
		return 0;
	}
	static legit::ioMouse::IOMouseType GetMouseX() {
		return MouseX;
	}
	static legit::ioMouse::IOMouseType GetMouseY() {
		return MouseY;
	}
private:
	static inline legit::ioMouse::IOMouseType MouseX = 0, MouseY = 0;

};
class CRawInput {
public:
	static bool IsNotAllowedToProcess(UINT msg, WPARAM wParam) {
		return msg != WM_INPUT || GetWParamCode(wParam) == RIM_INPUTSINK;
	}
	static legit::ioKey GetFromScanCode(uint16_t scanCode, bool extended) {
		switch (scanCode) {
			// ---------- Function row ----------
			case 0x01: return legit::ioKey::Escape;
			case 0x3B: return legit::ioKey::F1;
			case 0x3C: return legit::ioKey::F2;
			case 0x3D: return legit::ioKey::F3;
			case 0x3E: return legit::ioKey::F4;
			case 0x3F: return legit::ioKey::F5;
			case 0x40: return legit::ioKey::F6;
			case 0x41: return legit::ioKey::F7;
			case 0x42: return legit::ioKey::F8;
			case 0x43: return legit::ioKey::F9;
			case 0x44: return legit::ioKey::F10;
			case 0x57: return legit::ioKey::F11;
			case 0x58: return legit::ioKey::F12;

			// ---------- Number row ----------
			case 0x29: return legit::ioKey::Grave;
			case 0x02: return legit::ioKey::Key1;
			case 0x03: return legit::ioKey::Key2;
			case 0x04: return legit::ioKey::Key3;
			case 0x05: return legit::ioKey::Key4;
			case 0x06: return legit::ioKey::Key5;
			case 0x07: return legit::ioKey::Key6;
			case 0x08: return legit::ioKey::Key7;
			case 0x09: return legit::ioKey::Key8;
			case 0x0A: return legit::ioKey::Key9;
			case 0x0B: return legit::ioKey::Key0;
			case 0x0C: return legit::ioKey::Minus;
			case 0x0D: return legit::ioKey::EqualsPlus;
			case 0x0E: return legit::ioKey::Backspace;

			// ---------- QWERTY ----------
			case 0x0F: return legit::ioKey::Tab;
			case 0x10: return legit::ioKey::Q;
			case 0x11: return legit::ioKey::W;
			case 0x12: return legit::ioKey::E;
			case 0x13: return legit::ioKey::R;
			case 0x14: return legit::ioKey::T;
			case 0x15: return legit::ioKey::Y;
			case 0x16: return legit::ioKey::U;
			case 0x17: return legit::ioKey::I;
			case 0x18: return legit::ioKey::O;
			case 0x19: return legit::ioKey::P;
			case 0x1A: return legit::ioKey::OpenBracket;
			case 0x1B: return legit::ioKey::CloseBracket;
			case 0x2B: return legit::ioKey::Backslash;

			// ---------- Home cluster ----------
			case 0x3A: return legit::ioKey::CapsLock;
			case 0x1E: return legit::ioKey::A;
			case 0x1F: return legit::ioKey::S;
			case 0x20: return legit::ioKey::D;
			case 0x21: return legit::ioKey::F;
			case 0x22: return legit::ioKey::G;
			case 0x23: return legit::ioKey::H;
			case 0x24: return legit::ioKey::J;
			case 0x25: return legit::ioKey::K;
			case 0x26: return legit::ioKey::L;
			case 0x27: return legit::ioKey::SemiColon;
			case 0x28: return legit::ioKey::Apostrophe;
			case 0x1C: return extended ? legit::ioKey::NumPadEnter : legit::ioKey::Enter;

			// ---------- Shift / Z-row ----------
			case 0x2A: return legit::ioKey::LShift;
			case 0x2C: return legit::ioKey::Z;
			case 0x2D: return legit::ioKey::X;
			case 0x2E: return legit::ioKey::C;
			case 0x2F: return legit::ioKey::V;
			case 0x30: return legit::ioKey::B;
			case 0x31: return legit::ioKey::N;
			case 0x32: return legit::ioKey::M;
			case 0x33: return legit::ioKey::Comma;
			case 0x34: return legit::ioKey::Period;
			case 0x35: return extended ? legit::ioKey::NumPadDivide : legit::ioKey::Slash;
			case 0x36: return legit::ioKey::RShift;

			// ---------- Ctrl / Alt / Win ----------
			case 0x1D: return extended ? legit::ioKey::RControl : legit::ioKey::LControl;
			case 0x38: return extended ? legit::ioKey::RAlt : legit::ioKey::LAlt;
			case 0x5B: return legit::ioKey::LWin;
			case 0x5C: return legit::ioKey::RWin;
			case 0x5D: return legit::ioKey::Menu;

			// ---------- Arrow / Nav cluster ----------
			case 0x47: return extended ? legit::ioKey::Home : legit::ioKey::NumPad7;
			case 0x48: return extended ? legit::ioKey::Up : legit::ioKey::NumPad8;
			case 0x49: return extended ? legit::ioKey::PageUp : legit::ioKey::NumPad9;
			case 0x4B: return extended ? legit::ioKey::Left : legit::ioKey::NumPad4;
			case 0x4D: return extended ? legit::ioKey::Right : legit::ioKey::NumPad6;
			case 0x4F: return extended ? legit::ioKey::End : legit::ioKey::NumPad1;
			case 0x50: return extended ? legit::ioKey::Down : legit::ioKey::NumPad2;
			case 0x51: return extended ? legit::ioKey::PageDown : legit::ioKey::NumPad3;
			case 0x52: return extended ? legit::ioKey::Insert : legit::ioKey::NumPad0;
			case 0x53: return extended ? legit::ioKey::Delete : legit::ioKey::NumPadPeriod;

			// ---------- Numpad operators ----------
			case 0x4E: return legit::ioKey::NumPadAdd;
			case 0x4A: return legit::ioKey::NumPadMinus;
			case 0x6A: return legit::ioKey::NumPadMultiply;
			// ---------- Print / Pause / Lock ----------
			case 0x37: return legit::ioKey::PrintScreen;
			case 0xE1: return legit::ioKey::Pause;
			case 0x45: return legit::ioKey::NumPadLock; // NumLock
			case 0x46: return legit::ioKey::ScrollLock;

			default: return legit::ioKey::Unknown;
		}
	}
	static LRESULT WindowProc(HWND wnd, UINT m, WPARAM wParam, LPARAM lParam) {
		if (IsNotAllowedToProcess(m, wParam)) {
			return 0;
		}
		RAWINPUT raw = GetBuffer(lParam);
		if (raw.header.dwType == RIM_TYPEKEYBOARD) {
			bool isDown = !(raw.data.keyboard.Flags & RI_KEY_BREAK);
			legit::ioKey k = GetFromScanCode(raw.data.keyboard.MakeCode, raw.data.keyboard.Flags & RI_KEY_E0 || raw.data.keyboard.Flags & RI_KEY_E1);
			CRawInput::KbdData[(int)k].bIsKeyDown = isDown;
		} else if (raw.header.dwType == RIM_TYPEMOUSE) {
			CRawInput::MouseXDelta += raw.data.mouse.lLastX;
			CRawInput::MouseYDelta += raw.data.mouse.lLastY;
		}
		return 0;
	}
	static constexpr int KEY_MAX = (int)legit::ioKey::MAX_IO_KEY;
	static void Init(legit::ioKeyboard& kb, legit::ioMouse& mouse) {
		ApplicationWindowProcessor::AddWindowProc(WindowProc);
		KbdData = legit::ioHandler::GetKeyStateFromBoard(&kb);
		std::vector<RAWINPUTDEVICE> Devices{};
		RAWINPUTDEVICE raw{};
		raw.usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
		raw.usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
		raw.dwFlags = 0;    // adds mouse and also ignores legacy mouse messages
		raw.hwndTarget = 0;
		Devices.push_back(raw);
		raw.usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
		raw.usUsage = 0x06;              // HID_USAGE_GENERIC_KEYBOARD
		raw.dwFlags = 0;    // adds keyboard and also ignores legacy keyboard messages
		raw.hwndTarget = 0;
		Devices.push_back(raw);

		if (RegisterRawInputDevices(Devices.data(), Devices.size(), sizeof(RAWINPUTDEVICE)) == FALSE) {

		} else {
			printf("Successfully registered RawInputDevices\n");
		}
	}
	static void SetKey(legit::ioKey ke, bool IsDown) {
		KbdData[(int)ke].bIsKeyDown = IsDown;
	}
	static bool& GetKeyStatus(legit::ioKey ke) { // gonna have to do an assert to verify KbdData is actually a thing.
		return KbdData[(int)ke].bIsKeyDown;
	}
	static bool IsKeyDown(legit::ioKey key) {
		return KbdData[(int)key].bIsKeyDown;
	}
	static bool IsKeyUp(legit::ioKey key) {
		return !KbdData[(int)key].bIsKeyUp;
	}
	static void ResetMouseDeltas() {
		MouseXDelta = 0;
		MouseYDelta = 0;
	}
	static void Shutdown() {
		KbdData = nullptr;
	}
	static int MouseDeltaX() {
		return MouseXDelta;
	}
	static int MouseDeltaY() {
		return MouseYDelta;
	}
private:
	static RAWINPUT GetBuffer(LPARAM lParam) {
		HRESULT hResult{};
		UINT dwSize{};
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
		std::vector<BYTE> pRaw(dwSize);
		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, pRaw.data(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
			printf("get rawinputdata didn't return right size\n");
		}
		return RAWINPUT(*(RAWINPUT*)pRaw.data());
	}
	static char GetWParamCode(WPARAM wpar) {
		return GET_RAWINPUT_CODE_WPARAM(wpar);
	}
	static inline int MouseXDelta = 0;
	static inline int MouseYDelta = 0;
	static inline legit::ioKeyState* KbdData{nullptr};
};
namespace legit {
	template<typename T> void ZeroBlock(T* Pointer, legit::Size Count) {
		ZeroMemory(Pointer, sizeof(T) * Count);
	}
	template<typename T> void ZeroBlock(T* Pointer) {
		ZeroBlock(Pointer, 1);
	}
	template<typename T, size_t Size> void ZeroBlock(T(&array)[Size]) {
		ZeroMemory((array[0]), sizeof(T) * Size); // this just calls memset lmao. 
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

#include <Xinput.h>
#pragma comment(lib, "xinput.lib")
namespace legit {
	class XInput {
	public:
		struct sControllerInput {
			bool A = false, B = false, X = false, Y = false, LB = false, RB = false, RJoyButton = false, LJoyButton = false, Start = false, Back = false, DPadUp = false, DPadDown = false, DPadRight = false, DPadLeft = false;
			short LT = 0, RT = 0;
			short RightStickX = 0, LeftStickX = 0;
			short RightStickY = 0, LeftStickY = 0;
		};
	private:
		struct Controller {
			sControllerInput Input;
			bool IsConnected = false;
			unsigned long long ConnectedTime = 0llu;
		};
	public:
		static void Init() {
			IsXInputEnabled = true;
		}
		static void Update(legit::ioGamePad& io) {
			// poll incoming devices.
			for (int i = 0; i < XUSER_MAX_COUNT; i++) {
				XINPUT_STATE state;
				legit::ZeroBlock(&state);
				int Result = XInputGetState((DWORD)i, &state); // so this constantly polls for new controllers. 
				Controller& ctr = m_Controllers[i];
				if (Result == ERROR_SUCCESS) {
					ctr.IsConnected = true;
					auto* v = legit::ioHandler::GetControllerValues(&io);
					CaptureButtons(v, state);
					Vibrate(i, io);
				} else {
					ctr.IsConnected = false;
				}
			}
		}
		static void Shutdown() {

		}
		static void Vibrate(int UserIndex, legit::ioGamePad& ioGamePad) {
			auto* lmotor = legit::ioHandler::GetControllerRumbleLeftMotor(&ioGamePad);
			auto* rmotor = legit::ioHandler::GetControllerRumbleRightMotor(&ioGamePad); // this might spam gamepad 
			if (*lmotor == legit::ioHandler::LastReportedMotorLValue || *rmotor == legit::ioHandler::LastReportedMotorRValue) return; // we don't need to set a value that already exists.
			XINPUT_VIBRATION vib{};
			vib.wLeftMotorSpeed = *lmotor;
			vib.wRightMotorSpeed = *rmotor;
			XInputSetState(UserIndex, &vib);

			legit::ioHandler::LastReportedMotorLValue = *lmotor;
			legit::ioHandler::LastReportedMotorRValue = *rmotor;
		}
		static void Vibrate(int Index, unsigned short LeftMotor, unsigned short RightMotor) {
			XINPUT_VIBRATION vibration{};
			vibration.wLeftMotorSpeed = LeftMotor;
			vibration.wRightMotorSpeed = RightMotor;
			XInputSetState(Index, &vibration);
		}
		static void CapturePressures(legit::ioGamePad* pGamePad, XINPUT_STATE& state) {
			// probably the dumbest way to do this, but whatever im in the architecture until I can rewrite it.
			for (int i = 0; i < (int)legit::ioGamePadPressureInputs::MAX_PRESSURES; i++) {
				auto* ptr = legit::ioHandler::GetControllerPressure(pGamePad, (legit::ioGamePadPressureInputs)i);
				switch (i) {
					case (int)legit::ioGamePadPressureInputs::LeftStickX:
						*ptr = state.Gamepad.sThumbLX;
						break;
					case (int)legit::ioGamePadPressureInputs::LeftStickY:
						*ptr = state.Gamepad.sThumbLY;
						break;
					case (int)legit::ioGamePadPressureInputs::RightStickX:
						*ptr = state.Gamepad.sThumbRX;
						break;
					case (int)legit::ioGamePadPressureInputs::RightStickY:
						*ptr = state.Gamepad.sThumbRY;
						break;
					case (int)legit::ioGamePadPressureInputs::LT:
						*ptr = state.Gamepad.bLeftTrigger;
						break;
					case (int)legit::ioGamePadPressureInputs::RT:
						*ptr = state.Gamepad.bRightTrigger;
						break;
				}
			}
		}
		static void CaptureButtons(bool* ctr, XINPUT_STATE& state) {
			auto Buttons = state.Gamepad.wButtons;
			WORD b = state.Gamepad.wButtons;
			ctr[(int)legit::ioGamePadButtons::A] = b & XINPUT_GAMEPAD_A;
			ctr[(int)legit::ioGamePadButtons::B] = b & XINPUT_GAMEPAD_B;
			ctr[(int)legit::ioGamePadButtons::X] = b & XINPUT_GAMEPAD_X;
			ctr[(int)legit::ioGamePadButtons::Y] = b & XINPUT_GAMEPAD_Y;

			ctr[(int)legit::ioGamePadButtons::LB] = b & XINPUT_GAMEPAD_LEFT_SHOULDER;
			ctr[(int)legit::ioGamePadButtons::RB] = b & XINPUT_GAMEPAD_RIGHT_SHOULDER;

			ctr[(int)legit::ioGamePadButtons::Back] = b & XINPUT_GAMEPAD_BACK;
			ctr[(int)legit::ioGamePadButtons::Start] = b & XINPUT_GAMEPAD_START;

			ctr[(int)legit::ioGamePadButtons::LJoy] = b & XINPUT_GAMEPAD_LEFT_THUMB;
			ctr[(int)legit::ioGamePadButtons::RJoy] = b & XINPUT_GAMEPAD_RIGHT_THUMB;

			ctr[(int)legit::ioGamePadButtons::DPadUp] = b & XINPUT_GAMEPAD_DPAD_UP;
			ctr[(int)legit::ioGamePadButtons::DPadDown] = b & XINPUT_GAMEPAD_DPAD_DOWN;
			ctr[(int)legit::ioGamePadButtons::DPadLeft] = b & XINPUT_GAMEPAD_DPAD_LEFT;
			ctr[(int)legit::ioGamePadButtons::DPadRight] = b & XINPUT_GAMEPAD_DPAD_RIGHT;

		}
		static inline bool IsXInputEnabled = false;
		static inline std::array<Controller, XUSER_MAX_COUNT> m_Controllers;
	};
}
namespace legit {
	class ioInput {
	public:
		static void Init() {
			mMouse = new legit::ioMouse();
			mKeyboard = new legit::ioKeyboard();
			mGamePad = new legit::ioGamePad();
			CIOBasis::Init();
			CRawInput::Init(*mKeyboard, *mMouse);
			XInput::Init();
		}
		static legit::ioMouse& GetMouse() {
			return *mMouse;
		}
		static legit::ioKeyboard& GetKeyboard() {
			return *mKeyboard;
		}
		static legit::ioGamePad& GetGamePad() {
			return *mGamePad;
		}
		static void Update() {
			UpdateMouse();
			UpdateKeyboard();
			XInput::Update(*mGamePad);
		}
		static void Shutdown() {
			CRawInput::Shutdown();
			XInput::Shutdown();
			legit::Delete(mGamePad);
			legit::Delete(mKeyboard);
			legit::Delete(mMouse);
		}
	private:
		static void UpdateMouse() {
			if (!mMouse) return;
			float x = float(CIOBasis::GetMouseX()) / float(ApplicationWindowProcessor::Get().WindowWidth);
			float y = float(CIOBasis::GetMouseY()) / float(ApplicationWindowProcessor::Get().WindowHeight);
			legit::ioHandler::SetMousePrivate(mMouse, CRawInput::MouseDeltaX(), CRawInput::MouseDeltaY(), CIOBasis::GetMouseX(), CIOBasis::GetMouseY(), x, y);
			CRawInput::ResetMouseDeltas(); // mouse deltas breaks functionality (as end frame is weird) 
		}
		static void UpdateKeyboard() {
			if (!mKeyboard)return;
			for (int i = 0; i < CRawInput::KEY_MAX; i++) {
				auto status = CRawInput::GetKeyStatus((legit::ioKey)i);
				legit::ioKeyState state{};
				state.bIsKeyDown = status;
				state.bIsKeyUp = !status; // kinda useless lmao.
				legit::ioHandler::SetKey(mKeyboard, (legit::ioKey)i, state);
			}
		}
		static inline legit::ioMouse* mMouse = nullptr;
		static inline legit::ioKeyboard* mKeyboard = nullptr;
		static inline legit::ioGamePad* mGamePad = nullptr;
	};
}
class CAppl {
public:
	static void Init() {
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
		HWND hwnd = CreateWindowEx(
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
		if (hwnd == NULL) {
			printf("Failed to create window %d\n", GetLastError());
			__debugbreak();
			return;
		}
		ShowWindow(hwnd, SW_SHOW);
		legit::ioInput::Init();
	}
	static void Update() {
		while (!ApplicationWindowProcessor::Get().IsCloseRequested) {
			if (legit::ioInput::GetKeyboard().IsDown(legit::ioKey::A)) {
				legit::ioInput::GetGamePad().Vibrate(legit::MAX_USHORT);
			} else {
				legit::ioInput::GetGamePad().Vibrate(0);
			}
			ApplicationWindowProcessor::Update();
			legit::ioInput::Update();
		}
	}
	static void Shutdown() {
		legit::ioInput::Shutdown();
	}
private:
	void* Window = nullptr;
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

