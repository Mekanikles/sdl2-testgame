#pragma once

#include "Core.h"

namespace jcpe
{

#if defined(__IPHONEOS__) || defined(__ANDROID__)
	#define SUPPORTS_OPENGLES
#endif

namespace graphics
{
	struct Window;
	struct Context;
	struct Program;

	/*
		Window creation/destruction
	*/

	struct WindowCreationParams
	{
		uint width;
		uint height;
	};

	owned_ptr<Window> createWindow(const WindowCreationParams& params);
	void destroyWindow(owned_ptr<Window> window);

	/*
		Context creation/destruction
	*/

	owned_ptr<Context> createContext(gsl::not_null<Window*> window);
	void destroyContext(owned_ptr<Context> context);

	/*
		Shader program creation/destruction
	*/

	struct ProgramCreationParams
	{
		gsl::not_null<const char*> vertexSource;
		gsl::not_null<const char*> fragmentSource;
	};

	owned_ptr<Program> createProgram(const ProgramCreationParams& params);
	void destroyProgram(owned_ptr<Program> program);

	/*
		Frame functions
	*/

	void swapBuffers(gsl::not_null<Window*> window);

	/*
		Rendering primitive functions
	*/

	void setClearColor(const ColorRGBA& color);
	void clearFrameBuffer();

} // namespace graphics

}