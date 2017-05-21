#pragma once

#include <sstream>
#include <ostream>
#include <iostream>
#include <thread>

#include "SDL_log.h"

namespace jcpe
{

extern unsigned int s_frame;

}

#ifndef RELEASE
	#define LOG_RAW( text )																		   \
		do																						   \
		{																	   					   \
			std::cerr << text;																	   \
		}																						   \
		while(0)

	#define LOG( text )																			   \
		do																						   \
		{																						   \
			auto tid = std::this_thread::get_id();												   \
			std::ostringstream os;																   \
			os << __FUNCTION__ << "(" << __LINE__ << "), Thread " << tid <<						   \
					", frame " << jcpe::s_frame << ":" << std::endl << "    " << 				   \
					text << std::endl;															   \
			SDL_Log("%s", os.str().c_str());                                                       \
		}																						   \
		while(0)

	#define FATAL_ASSERT_DESC( statement, desc )												   \
		do																						   \
		{																						   \
			if (!( statement ))																	   \
			{																					   \
				LOG("Fatal assertion failed: '" << desc << "'");								   \
				throw std::runtime_error(std::string(desc));									   \
			}																					   \
		}																						   \
		while(0)
#else
	#define LOG( text ) {}
	#define FATAL_ASSERT_DESC( statement , desc) {}
#endif

#define FATAL_ASSERT( statement ) FATAL_ASSERT_DESC( statement, #statement )

// For now, treat non-fatal asserts the same
#define ASSERT_DESC( statement, desc) FATAL_ASSERT_DESC( statement, desc)
#define ASSERT( statement ) FATAL_ASSERT( statement )

