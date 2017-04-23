#pragma once

#if defined(__WINDOWS__)
	#if _MSC_VER==1900
		#define USING_VS2015
	#endif
#endif

#if !defined(__WINDOWS__) || defined(USING_VS2015)
	#define SUPPORTS_CPP11_CONSTRUCTOR_INHERIT
#endif
