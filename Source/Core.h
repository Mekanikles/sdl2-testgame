#pragma once


#include <cassert>
#include <cstddef>
#include <memory>

#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl>

#include "Log.h"
#include "CoreTypes.h"

// Size of statically allocated array, courtesy of Google v8.
#define array_size(a) ((sizeof(a) / sizeof(*(a))) / static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))


template <typename T>
struct Assert_On_Not_Null_Deleter
{
	void operator() (T* t) { assert(t == nullptr && "Owned pointer was not released before owner deletion"); }
};

template <typename T>
class owned_ptr : public std::unique_ptr<T, Assert_On_Not_Null_Deleter<T> >
{
public:
	using std::unique_ptr<T, Assert_On_Not_Null_Deleter<T> >::unique_ptr;

	operator gsl::not_null<T*>() { return gsl::not_null<T*>(get()); }
	operator T*() const { return get(); }

	bool hasOwnership() { return get() != nullptr; }
};

// ScopeExit functionality, courtesy of the-witness.net
template <typename FuncT>
class ScopeExit 
{
public:
	ScopeExit(FuncT f) : m_f(f) {}
	~ScopeExit() { m_f(); }
private:
	FuncT m_f;
};

template <typename FuncT>
ScopeExit<FuncT> makeScopeExit(FuncT f) 
{
	return ScopeExit<FuncT>(f);
};

#define INTERNAL_CONCAT(x, y) x ## y
#define CONCAT(x, y) INTERNAL_CONCAT(x, y)
#define AT_SCOPE_EXIT(code) auto CONCAT(at_scope_exit_, __LINE__) = makeScopeExit([&](){code;})
