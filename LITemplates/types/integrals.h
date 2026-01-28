#pragma once
namespace legit {
	static constexpr const unsigned long long DefaultCharSize = 1llu;
	static constexpr const unsigned long long DefaultShortSize = 2llu;
	static constexpr const unsigned long long DefaultLongSize = 4llu;
	static constexpr const unsigned long long DefaultLong64Size = 8llu;


	using u8 = unsigned char;
	using s8 = signed char;
	using u16 = unsigned short;
	using s16 = signed short;
	using u32 = unsigned long;
	using s32 = signed long;
	using u64 = unsigned long long;
	using s64 = signed long long;

	static_assert(sizeof(u8) == DefaultCharSize);
	static_assert(sizeof(u16) == DefaultShortSize);
	static_assert(sizeof(u32) == DefaultLongSize);
	static_assert(sizeof(u64) == DefaultLong64Size);
}