#pragma once

#include <cassert>
#include <cstddef>
#include <memory>

#include "lang.h"
#include "Log.h"

#include "glm/gtc/type_ptr.hpp"

using namespace lang;

struct Color32 : public vec4
{
	Color32() = default;

	Color32(uint uColor) : vec4(math::unpackUnorm4x8(uColor))
	{}

	Color32(float r, float b, float g) : vec4(r, g, b, 1.0f)
	{}

	Color32(float r, float b, float g, float a) : vec4(r, g, b, a)
	{}

	vec4u8 bytes() const
	{
		auto pack = math::packUnorm4x8(*this);
		const uint8* byteArr = (uint8*)&pack;
		return glm::make_vec4(byteArr);
	}
};

// Can be used for strongly typing "typedefs"
//	like so: using MyType = TypeWrapper<T>;
template<typename T, bool E = std::is_fundamental<T>::value>
struct TypeWrapper;

template<typename T>
struct TypeWrapper<T, false> : public T
{
	using T::T;
};

template<typename T>
struct TypeWrapper<T, true>
{
	template <typename... Args>
	explicit TypeWrapper(Args&&... args)
		: value(std::forward<Args>(args)...)
	{}

	TypeWrapper& operator=(const T& v) { value = v; return *this; }

    operator T&() { return value; }
    operator T() const { return value; }

	T value;
};

template<typename T>
using TPoint2 = TypeWrapper<tvec2<T>>;

template<typename T>
struct TRect2
{
	union
	{
		tvec4<T> vec;
		struct { TPoint2<T> p1; TPoint2<T> p2; };
		struct { T x1; T y1; T x2; T y2; };
		struct { T x; T y; };
	};

	TRect2(const TPoint2<T>& p1, const TPoint2<T>& p2) : p1(p1), p2(p2) {}
	TRect2(const TPoint2<T>& p, const tvec2<T>& s) : p1(p), p2(p + s) {}
	TRect2(const TPoint2<T>& p, const T& width, const T& height) : p1(p), p2(p.x + width, p.y + height) {}	

	T width() const { return x2 - x1; }
	T height() const { return y2 - y1; }
	tvec2<T> size() const { return p2 - p1; }
};

using Rect2 = TRect2<float>;
using Rect2i = TRect2<int>;
using Point2 = TPoint2<float>;
using Point2i = TPoint2<int>;

