/*#define GLEW_STATIC
#include <GL/glew.h>

static_assert(GL_VERSION_2_0, "System must support opengl 2.0");
*/

/*
#if defined(__IPHONEOS__) || defined(__ANDROID__)
	#define SUPPORTS_OPENGLES
#endif

#ifdef SUPPORTS_OPENGLES
	#include <SDL_opengles2.h>
#else
	#include <SDL_opengl.h>
#endif
*/

/*
namespace
{
// Shader sources
const GLchar* vertexSource = R"(
	attribute vec4 position;
	void main()
	{
		gl_Position = vec4(position.xyz, 1.0);
	}
	)";

const GLchar* fragmentSource = R"(
	precision mediump float;
	void main()
	{
		gl_FragColor = vec4 (1.0, 1.0, 1.0, 1.0 );
	}
	)";

}
*/

#include <type_traits>

static_assert(std::is_trivially_copyable<const char*>(), "dasfa");



#include "Graphics.h"

#include "Core.h"

#define GLEW_STATIC
#include <GL/glew.h>

#include "SDL.h"


#ifdef SUPPORTS_OPENGLES
	#include <SDL_opengles2.h>
#else
	#include <SDL_opengl.h>
#endif

namespace jcpe
{

namespace graphics
{

///////////////////////////////////////////////////////////////////////////////////////////////////

struct Window
{
	SDL_Window* sdlWindow;
};

struct Context
{
	SDL_GLContext sdlContext;
};

struct Program
{
	GLuint glProgramObject;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

owned_ptr<Window> createWindow(const WindowCreationParams& params)
{
	SDL_Window* sdlWindow;

    // Create an application window with the following settings:
    sdlWindow = SDL_CreateWindow(
        "An SDL2 window",                  // window title
        SDL_WINDOWPOS_CENTERED,           // initial x position
        SDL_WINDOWPOS_CENTERED,           // initial y position
        params.width,
        params.height,
        SDL_WINDOW_OPENGL                  // flags
    );

    // Check that the window was successfully created
    if (sdlWindow == NULL) 
	{
        // In the case that the window could not be made...
        LOG("Could not create window: " <<  SDL_GetError());
        return nullptr;
    }

	Window* window = new Window { sdlWindow };
	return owned_ptr<Window>(window);
}

void destroyWindow(owned_ptr<Window> window)
{
	assert(window.hasOwnership() && "Trying to destroy null window");

	Window* windowptr = window.release();
    SDL_DestroyWindow(windowptr->sdlWindow);

	delete(windowptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

owned_ptr<Context> createContext(gsl::not_null<Window*> window)
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetSwapInterval(1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	
    SDL_GLContext sdlc = SDL_GL_CreateContext(window->sdlWindow);

	glewExperimental = GL_FALSE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		LOG("Could not create context: " << glewGetErrorString(err));
		return nullptr;
	}

	Context* context = new Context{ sdlc };
	return owned_ptr<Context>(context);
}

void destroyContext(owned_ptr<Context> context)
{
	assert(context.hasOwnership() && "Trying to destroy null context");

	Context* contextptr = context.release();
	SDL_GL_DeleteContext(contextptr->sdlContext);

	delete(contextptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static GLuint loadShader(const char *shaderSrc, GLenum type)
{
	GLuint shader;
	GLint compiled;
	// Create the shader object
	shader = glCreateShader(type);
	if(shader == 0)
		return 0;
	// Load the shader source
	glShaderSource(shader, 1, &shaderSrc, NULL);
	// Compile the shader
	glCompileShader(shader);
	// Check the compile status
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if(!compiled)
	{
		char buffer[4096];
		glGetShaderInfoLog(shader, 4096, NULL, buffer);
		LOG("Error compiling chader: " << buffer);

		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

owned_ptr<Program> createProgram(const ProgramCreationParams& params)
{
	// Load the vertex/fragment shaders
	const GLuint vertexShader = loadShader(params.vertexSource, GL_VERTEX_SHADER);
	const GLuint fragmentShader = loadShader(params.fragmentSource, GL_FRAGMENT_SHADER);

	if (!vertexShader || !fragmentShader)
		return nullptr;

	// Create the program object
	GLuint programObject = glCreateProgram();

	if(programObject == 0)
		return nullptr;

	glAttachShader(programObject, vertexShader);
	glAttachShader(programObject, fragmentShader);

	// Link the program
	glLinkProgram(programObject);

	// Check the link status
	GLint linked;
	glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
	if(!linked)
	{
		char buffer[16384];
		glGetProgramInfoLog(programObject, array_size(buffer), NULL, buffer);
		LOG("Error linking program: " << buffer);

		glDeleteProgram(programObject);
		return nullptr;
	}

	Program* program = new Program { programObject };
	return owned_ptr<Program>(program);
}

void destroyProgram(owned_ptr<Program> program)
{
	assert(program.hasOwnership() && "Trying to destroy null program");

	Program* programptr = program.release();
    glDeleteProgram(programptr->glProgramObject);

	delete(programptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void swapBuffers(gsl::not_null<Window*> window)
{
	SDL_GL_SwapWindow(window->sdlWindow);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void setClearColor(const ColorRGBA& color)
{
	glClearColor(color.x, color.y, color.z, color.a);
}

void clearFrameBuffer()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

} // namespace graphics

}