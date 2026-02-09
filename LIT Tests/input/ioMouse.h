#pragma once
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
}
