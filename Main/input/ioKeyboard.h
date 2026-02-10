#pragma once
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
		ioKeyboard();
		ioKeyState GetState(legit::ioKey key);
		bool IsDown(legit::ioKey key) const;
		bool IsUp(legit::ioKey key) const;
	private:
		ioKeyState KeyState[int(ioKey::MAX_IO_KEY)]{};
	};
}
