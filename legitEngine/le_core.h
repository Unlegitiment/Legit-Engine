#pragma once
class legitEngine {
public:
	static void InitClass() {
		sm_LegitEngine = new legitEngine();
	}
	static void ShutdownClass() {
		delete sm_LegitEngine;
		sm_LegitEngine = nullptr;
	}
private:
	static inline legitEngine* sm_LegitEngine = nullptr;
	legitEngine() {
		
	}
};