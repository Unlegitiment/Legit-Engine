#pragma once
#include <immintrin.h>
struct FloatInstructions {
	static __m128 Addf(__m128 A, __m128 B) {
		return _mm_add_ps(A, B);
	}
	static __m128 Subf(__m128 A, __m128 B) {
		return _mm_sub_ps(A, B);
	}
	static __m128 Mulf(__m128 A, __m128 B) {
		return _mm_mul_ps(A, B);
	}
	static __m128 Divf(__m128 A, __m128 B) {
		return _mm_div_ps(A, B);
	}
	static __m128 Orf(__m128 A, __m128 B) {
		return _mm_or_ps(A, B);
	}
	static __m128 Andf(__m128 A, __m128 B) {
		return _mm_and_ps(A, B);
	}
	static __m128 Xorf(__m128 A, __m128 B) {
		return _mm_xor_ps(A, B);
	}
	static __m128 Setf(float x,float y,float z,float w) {
		return _mm_set_ps(x,y,z,w);
	}
	static void Extractf(__m128 A, float& x, float& y, float& z, float& w) {
		float Components[4] = {};
		_mm_storeu_ps(Components, A);
		x = Components[0];
		y = Components[1];
		z = Components[2];
		w = Components[3];
	}
};
class leVec4 {
public:
	using Value128 = __m128;
	using Val = Value128;
	leVec4(Val v) {
		Data = v;
	}
	leVec4(float x, float y, float z, float w) {
		float f[4] = {x, y, z, w};
		Data = _mm_loadu_ps(f);
	}
	leVec4 operator+(const leVec4& op) {
		return _mm_add_ps(this->Data, op.Data);
	}
	leVec4 operator/(const leVec4& op) {
		return _mm_div_ps(this->Data, op.Data);
	}
	leVec4 operator*(const leVec4& op) {
		return _mm_mul_ps(this->Data, op.Data);
	}
	leVec4 operator-(const leVec4& op) {
		return _mm_sub_ps(this->Data, op.Data);
	}
	Val& Get() {
		return this->Data;
	}
	template<size_t B> void AsByte(char (&Name)[B]) const {}
	template<> void AsByte<16>(char(&Name)[16]) const {
		float x, y, z, w;
		FloatInstructions::Extractf(Data, x,y,z,w);
		Name[0] = x;
		Name[1] = (char)x >> 8;
		Name[2] = (char)x >> 16;
		Name[3] = (char)x >> 32; //  4 bytes
		Name[4] = y;
		Name[5] = (char)y >> 8;
		Name[6] = (char)y >> 16;
		Name[7] = (char)y >> 32; //  4 bytes
		Name[8] = z;
		Name[9] = (char)z >> 8;
		Name[10] = (char)z >> 16;
		Name[11] = (char)z >> 32; //  4 bytes
		Name[12] = w;
		Name[13] = (char)w >> 8;
		Name[14] = (char)w >> 16;
		Name[15] = (char)w >> 32; //  4 bytes
	}
private:
	Val Data;
};