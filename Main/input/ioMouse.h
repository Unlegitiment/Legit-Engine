#pragma once
namespace legit {
	class ioMouse {
	public:
		friend class ioHandler;
		using IOMouseType = int;
		using DeltaType = int;
		using MouseNormalized = float;
		DeltaType GetDeltaX() const;
		DeltaType GetDeltaY() const;
		IOMouseType GetX() const;
		IOMouseType GetY() const;
		MouseNormalized GetNormX() const;
		MouseNormalized GetNormY() const;
	private:
		DeltaType MouseXDelta;
		DeltaType MouseYDelta;
		IOMouseType MouseX;
		IOMouseType MouseY;
		MouseNormalized MouseNormalizedX;
		MouseNormalized MouseNormalizedY;
	};
}
