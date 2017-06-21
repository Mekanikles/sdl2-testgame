#pragma once

#include <cassert>
#include <memory>
#include <vector>
#include <stdint.h>

#include "platform.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#define GSL_THROW_ON_CONTRACT_VIOLATION
#include "gsl/gsl"

// TODO: Move out to math lib
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/norm.hpp"

namespace lang
{
	// TODO: Generalize with value traits base class<
	template<typename T>
	class positive_numerical
	{
		positive_numerical(const T& v)
			: m_value(v)
		{
			Expects(v >= 0);
		}

    	constexpr operator T() const { return m_value; }		

	private:
		T m_value;
	};

	// TODO: Math lib instead of lang?
	namespace math = glm;

	using int8 = int8_t;
	using int16 = int16_t;
	using int32 = int32_t;
	using int64 = int64_t;
	using uint8 = uint8_t;
	using uint16 = uint16_t;
	using uint32 = uint32_t;
	using uint64 = uint64_t;
	using uint = unsigned int;

	using std::string;
	using std::vector;
	using std::array;

	using glm::vec2;
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	using glm::tvec2;
	using glm::tvec3;
	using glm::tvec4;

	using vec2i = tvec2<int>;
	using vec3i = tvec3<int>;
	using vec4i = tvec4<int>;

	using vec2u8 = tvec2<uint8>;
	using vec3u8 = tvec3<uint8>;
	using vec4u8 = tvec4<uint8>;

	using vec2u16 = tvec2<uint16>;
	using vec3u16 = tvec3<uint16>;
	using vec4u16 = tvec4<uint16>;

	using vec2u32 = tvec2<uint32>;
	using vec3u32 = tvec3<uint32>;
	using vec4u32 = tvec4<uint32>;

	using vec2i8 = tvec2<int8>;
	using vec3i8 = tvec3<int8>;
	using vec4i8 = tvec4<int8>;

	using vec2i16 = tvec2<int16>;
	using vec3i16 = tvec3<int16>;
	using vec4i16 = tvec4<int16>;

	using vec2i32 = tvec2<int32>;
	using vec3i32 = tvec3<int32>;
	using vec4i32 = tvec4<int32>;

	using gsl::span;
	using gsl::not_null;

	#define array_size(a) ((sizeof(a) / sizeof(*(a))) / static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

	template <typename T, class Deleter = std::default_delete<T>>
	class unique_ptr : public std::unique_ptr<T, Deleter>
	{
	    using Super_t = std::unique_ptr<T, Deleter>;
	public:
		
		// Forward base constructors
	#ifdef SUPPORTS_CPP11_CONSTRUCTOR_INHERIT
		using std::unique_ptr<T, Deleter>::unique_ptr;
	#else
		template <typename... Args>
		unique_ptr(Args&&... args)
			: Super_t(std::forward<Args>(args)...)
		{}
	#endif

		// Allow implicit conversion to reduce code clutter
		operator not_null<T*>() { return not_null<T*>(Super_t::get()); }
		operator const T*() const { return Super_t::get(); }
	};

	template<typename T, typename... Args>
	unique_ptr<T> make_unique(Args&&... args)
	{
		return unique_ptr<T>(new T(std::forward<Args>(args)...));
	}

	template <typename T>
	struct Assert_On_Not_Null_Deleter
	{
	       void operator() (T* t) { assert(t == nullptr && "Owned pointer was not released before owner deletion"); }
	};

	// TODO: Try using unique_ptr instead
	template <typename T>
	using owned_ptr = unique_ptr<T, Assert_On_Not_Null_Deleter<T>>;	

	template <typename FuncT>
	class scope_exit 
	{
	public:
		scope_exit(FuncT f) : m_f(f) {}
		~scope_exit() { m_f(); }
	private:
		FuncT m_f;
	};

	template <typename FuncT>
	scope_exit<FuncT> make_scope_exit(FuncT f) 
	{
		return scope_exit<FuncT>(f);
	};

	#define INTERNAL_CONCAT(x, y) x ## y
	#define CONCAT(x, y) INTERNAL_CONCAT(x, y)
 
	#define SCOPE_EXIT(code) auto CONCAT(on_scope_exit_, __LINE__) = make_scope_exit([&](){code})

	#define UNIQUE_SYMBOL(x) CONCAT(x, __COUNTER__)

















}