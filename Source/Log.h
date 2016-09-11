#pragma once

#include <sstream>
#include <ostream>
#include <iostream>

#include "SDL_log.h"

namespace jcpe
{

#ifndef RELEASE
	#define LOG( text )																								\
		do																											\
		{																											\
			std::ostringstream os;																					\
			os << __FUNCTION__ << "(" << __LINE__ << "): " << text << std::endl;									\
			SDL_Log("%s", os.str().c_str());                                                                        \
		}																											\
		while(0)

	#define FATAL_ASSERT_DESC( statement, desc )																	\
		do																											\
		{																											\
			if (!( statement ))																						\
			{																										\
				LOG("Fatal assert: " << desc);																		\
				throw std::runtime_error(std::string(desc));														\
			}																										\
		}																											\
		while(0)
#else
	#define LOG( text ) {}
	#define FATAL_ASSERT_DESC( statement , desc) {}
#endif

#define FATAL_ASSERT( statement ) FATAL_ASSERT_DESC( statement, #statement )

// For now, treat non-fatal asserts the same
#define ASSERT_DESC( statement, desc) FATAL_ASSERT_DESC( statement, desc)
#define ASSERT( statement ) FATAL_ASSERT( statement )


}

