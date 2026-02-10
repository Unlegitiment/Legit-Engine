#include "raw_input.h"
#include <WinUser.h>
#include <vector>
bool legit::CRawInput::IsNotAllowedToProcess(UINT msg, WPARAM wParam) {
	return msg != WM_INPUT || GetWParamCode(wParam) == RIM_INPUTSINK;
}

legit::ioKey legit::CRawInput::GetFromScanCode(unsigned short scanCode, bool extended) {
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

LRESULT legit::CRawInput::WindowProc(HWND wnd, UINT m, WPARAM wParam, LPARAM lParam) {
	if (IsNotAllowedToProcess(m, wParam)) {
		return 0;
	}
	RAWINPUT raw = GetBuffer(lParam);
	if (raw.header.dwType == RIM_TYPEKEYBOARD) {
		bool isDown = !(raw.data.keyboard.Flags & RI_KEY_BREAK);
		legit::ioKey k = GetFromScanCode(raw.data.keyboard.MakeCode, raw.data.keyboard.Flags & RI_KEY_E0 || raw.data.keyboard.Flags & RI_KEY_E1);
		legit::CRawInput::KbdData[(int)k].bIsKeyDown = isDown;
	} else if (raw.header.dwType == RIM_TYPEMOUSE) {
		legit::CRawInput::MouseXDelta += raw.data.mouse.lLastX;
		legit::CRawInput::MouseYDelta += raw.data.mouse.lLastY;
	}
	return 0;
}

void legit::CRawInput::Init(void(*AddToEventHandler)(WNDPROC), legit::ioKeyboard& kb, legit::ioMouse& mouse) {
	AddToEventHandler(WindowProc);
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

void legit::CRawInput::SetKey(legit::ioKey ke, bool IsDown) {
	KbdData[(int)ke].bIsKeyDown = IsDown;
}

bool& legit::CRawInput::GetKeyStatus(legit::ioKey ke) { // gonna have to do an assert to verify KbdData is actually a thing.
	return KbdData[(int)ke].bIsKeyDown;
}

bool legit::CRawInput::IsKeyDown(legit::ioKey key) {
	return KbdData[(int)key].bIsKeyDown;
}

bool legit::CRawInput::IsKeyUp(legit::ioKey key) {
	return !KbdData[(int)key].bIsKeyUp;
}

void legit::CRawInput::ResetMouseDeltas() {
	MouseXDelta = 0;
	MouseYDelta = 0;
}

void legit::CRawInput::Shutdown() {
	KbdData = nullptr;

}

int legit::CRawInput::MouseDeltaX() {
	return MouseXDelta;
}

int legit::CRawInput::MouseDeltaY() {
	return MouseYDelta;
}

RAWINPUT legit::CRawInput::GetBuffer(LPARAM lParam) {
	HRESULT hResult{};
	UINT dwSize{};
	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	std::vector<BYTE> pRaw(dwSize);
	if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, pRaw.data(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
		printf("get rawinputdata didn't return right size\n");
	}
	return RAWINPUT(*(RAWINPUT*)pRaw.data());
}

char legit::CRawInput::GetWParamCode(WPARAM wpar) {
	return GET_RAWINPUT_CODE_WPARAM(wpar);
}
