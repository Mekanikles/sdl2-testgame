#pragma once

#include <cassert>
#include <memory>

#include "platform.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#define GSL_THROW_ON_CONTRACT_VIOLATION
#include "gsl/gsl"

namespace lang
{
	using uint = unsigned;

	using glm::vec2;
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	#define array_size(a) ((sizeof(a) / sizeof(*(a))) / static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

	template<typename T>
	using not_null = gsl::not_null<T>;

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

	template <typename T>
	struct Assert_On_Not_Null_Deleter
	{
	       void operator() (T* t) { assert(t == nullptr && "Owned pointer was not released before owner deletion"); }
	};

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

















}