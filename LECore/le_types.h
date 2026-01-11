#pragma once
#define CompilerAssert(x) static_assert(x)
#define Likely(x) (x)
#define Unlikely(x) (x)
namespace legit {
	using s8 = signed char;
	using s16 = signed short;
	using s32 = signed long;
	using s64 = signed long long;
	using u8 = unsigned char;
	using u16 = unsigned short;
	using u32 = unsigned long;
	using u64 = unsigned long long;
	using byte = char; // No Sign/Unsign rep.
	using b8 = bool;
	using pByte = byte*;
	using pVoid = void*;
	CompilerAssert(sizeof(pVoid) == 8 && "Compiler not set to 64-bit");
}
#define RBlack		"\033[30m"  // Black Text
#define RRed		"\033[31m"  // Red Text
#define RGreen		"\033[32m"  // Green Text
#define RYellow		"\033[33m"  // Yellow Text
#define RBlue		"\033[34m"  // Blue Text
#define RPurple		"\033[35m"  // Purple Text
#define RCyan		"\033[36m"  // Cyan Text

#define RIRed		"\033[41m"  // Inversed Red Text
#define RIGreen		"\033[42m\033[30m"  // Inversed Green Text
#define RIYellow	"\033[43m\033[30m"  // Inversed Yellow Text
#define RIBlue		"\033[44m"  // Inversed Blue Text
#define RIPurple	"\033[45m"  // Inversed Purple Text
#define RICyan		"\033[46m"  // Inversed Cyan Text

#define RHIBlack		"\033[90m"			//Black
#define RHIRed			"\033[91m"			//Red
#define RHIGreen		"\033[92m"			//Green
#define RHIYellow		"\033[93m"			//Yellow
#define RHIBlue			"\033[94m"			//Blue
#define RHIPurple		"\033[95m"			//Purple
#define RHICyan			"\033[96m"			//Cyan
#define RHIWhite		"\033[97m"			//White

#define RNorm		"\033[0m"		// Normal Text