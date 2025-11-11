#pragma once
#include <intrin.h>
namespace legit {
	template<typename T> class _Vector2_ {
	public:
		using Type = T;
		using ConstType = const T;
		using TypePtr = T*;
		using TypeRef = T&;
		using TypeConstRef = const T&;
		using TypeConstPtr = const T*;

		_Vector2_() : x(0), y(0) {

		}
		_Vector2_(Type _x, Type _y) {
			this->x = _x;
			this->y = _y;
		}
		void NormalizeX(Type f) {
			x /= f;
		}
		void NormalizeY(Type f) {
			y /= f;
		}
		Type x, y;
	private:

	};
	using Vector2f = _Vector2_<float>;
	using Vector2i = _Vector2_<int>;
	// If this is a non-standard type it probably won't work. Which is fine :/
	template<typename T>
	class qVect2 {
	private:
		struct sRecvData {
			_Vector2_<T> m_Data;
		};
		static constexpr int XSLOT = 0;
		static constexpr int YSLOT = 1;
		using PtrType = T*;
	public:
		__m128 GetRaw() {
			return this->m_128;
		}
		_Vector2_<T> GetAsV2() {
			_Vector2_<T> ret;
			ret.x = Get()[XSLOT];
			ret.y = Get()[YSLOT];
			return ret;
		}
		void Put(T x, T y) {
			Get()[XSLOT] = x;
			Get()[YSLOT] = y;
		}
	private:
		PtrType& Get() {
			return GetVector<PtrType>();
		}
	private:
		template<typename R> R& GetVector() {
			return nullptr;
		}
		template<> float*& GetVector<float*>() {
			return m_128.m128_f32;
		}
		template<> int*& GetVector<int*>() {
			return m_128.m128_i32;
		}
		template<> short*& GetVector<short*>() {
			return m_128.m128_i16;
		}
		template<> char*& GetVector<char*>() {
			return m_128.m128_i8;
		}
		template<> long long*& GetVector<long long*>() {
			return m_128.m128_i64;
		}
		__m128 m_128; // 4 bit float? 32*4 
	};
	using qVect2f = qVect2<float>;
	using qVect2i = qVect2<int>;
}