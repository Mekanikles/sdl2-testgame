#include "Graphics.h"

#include "SDL.h"

#include "Core.h"

#if defined(__IPHONEOS__) || defined(__ANDROID__)
	#define SUPPORTS_OPENGLES
#endif

#ifdef SUPPORTS_OPENGLES
	#include "SDL_opengles2.h"
    #define USING_OPENGLES
#else
    #ifdef __WINDOWS__
        #define GLEW_STATIC
        #include "GL/glew.h"
        #define USING_GLEW
    #else
        // For now, ese gl3 as best match for ogles2
        #include "OpenGL/gl3.h"
    #endif
#endif

namespace jcpe
{

namespace Graphics
{

struct AttributeTypeInfo
{
	GLenum type;
	GLboolean normalized;
};

static AttributeTypeInfo s_attributeTypeInfos[] = 
	{ 	
		{ GL_FLOAT, GL_FALSE }, 			// AttributeType::Float
		{ GL_UNSIGNED_BYTE, GL_TRUE }, 		// AttributeType::NormalizedUByte
	};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct Window
{
	SDL_Window* sdlWindow;
	vec2i size;
	vec2 canvasSize;
};

struct Context
{
	SDL_GLContext sdlContext;
};

struct Program
{
	GLuint programHandle;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

owned_ptr<Window> createWindow(const WindowCreationParams& params)
{
	SDL_Window* sdlWindow;

    // Create an application window with the following settings:
    sdlWindow = SDL_CreateWindow(
        "SDL2 Test",                  // window title
        SDL_WINDOWPOS_CENTERED,           // initial x position
        SDL_WINDOWPOS_CENTERED,           // initial y position
        params.width,
        params.height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI                 // flags
    );

    // Check that the window was successfully created
    if (sdlWindow == NULL) 
	{
        // In the case that the window could not be made...
        LOG("Could not create window: " <<  SDL_GetError());
        return nullptr;
    }

    int canvasWidth;
    int canvasHeight;
    SDL_GetWindowSize(sdlWindow, &canvasWidth, &canvasHeight);
	int displayWidth;
	int displayHeight;
    SDL_GL_GetDrawableSize(sdlWindow, &displayWidth, &displayHeight);

	Window* window = new Window { sdlWindow, vec2i(displayWidth, displayHeight), vec2(canvasWidth, canvasHeight) };
	return owned_ptr<Window>(window);
}

void destroyWindow(owned_ptr<Window> window)
{
	Window* windowptr = window.release();
    SDL_DestroyWindow(windowptr->sdlWindow);

	delete(windowptr);
}

vec2i getWindowSize(not_null<const Window*> window)
{
	return window->size;
}

vec2 getWindowCanvasSize(not_null<const Window*> window)
{
	return window->canvasSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

owned_ptr<Context> createContext(not_null<Window*> window)
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetSwapInterval(1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	
    SDL_GLContext sdlc = SDL_GL_CreateContext(window->sdlWindow);

	const char* rendererDesc = (const char*)glGetString(GL_RENDERER);
	const char* glVersionDesc = (const char*)glGetString(GL_VERSION);

    #ifdef USING_GLEW
    {
        glewExperimental = GL_FALSE;
        GLenum err = glewInit();
        if (err != GLEW_OK)
        {
            LOG("Could not create context: " << glewGetErrorString(err));
			SDL_GL_DeleteContext(sdlc);
            return nullptr;
        }

		if (!GLEW_VERSION_3_2)
		{
            LOG("Could not create context. Incompatible OpenGL version: " << glVersionDesc);
			SDL_GL_DeleteContext(sdlc);
            return nullptr;
		}
    }
    #endif

	LOG("Created context with renderer: " << rendererDesc << ", OpenGL version: " << glVersionDesc); 

	Context* context = new Context{ sdlc };
	return owned_ptr<Context>(context);
}

void destroyContext(owned_ptr<Context> context)
{
	Context* contextptr = context.release();
	SDL_GL_DeleteContext(contextptr->sdlContext);

	delete(contextptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static GLuint loadShader(const char *shaderSrc, GLenum type)
{
	// Create the shader object
	const GLuint shader = glCreateShader(type);
	if(shader == 0)
		return 0;

	// Load the shader source
	glShaderSource(shader, 1, &shaderSrc, NULL);
	// Compile the shader
	glCompileShader(shader);
	// Check the compile status
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if(compiled != GL_TRUE)
	{
		char buffer[4096];
		glGetShaderInfoLog(shader, 4096, NULL, buffer);
		LOG("Error compiling chader: " << buffer);

		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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

	// Clean up
	glDetachShader(programObject, vertexShader);
	glDetachShader(programObject, fragmentShader);	
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);	

	// Check the link status
	GLint linked;
	glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
	if(linked != GL_TRUE)
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
	Program* programPtr = program.release();
    glDeleteProgram(programPtr->programHandle);

	delete(programPtr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

UniformHandle fetchUniformHandle(not_null<const Program*> program, const string& name)
{
	const GLuint handle = glGetUniformLocation(program->programHandle, name.c_str());
	return (UniformHandle)handle;
}

AttributeHandle fetchAttributeHandle(not_null<const Program*> program, const string& name)
{
	const GLuint handle = glGetAttribLocation(program->programHandle, name.c_str());
	return UniformHandle(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BufferHandle createBuffer()
{
	GLuint handle;
	glGenBuffers(1, &handle);
	return (BufferHandle)handle;
}

void destroyBuffer(const BufferHandle& buffer)
{
	glDeleteBuffers(1, (const GLuint*)&buffer);
}

void* createAndMapVertexBufferData(const BufferHandle& buffer, const uint capacity)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, capacity, NULL, GL_STREAM_DRAW);
	return glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void* createAndMapIndexBufferData(const BufferHandle& buffer, const uint capacity)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, capacity, NULL, GL_STREAM_DRAW);
	return glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void unmapVertexBufferData()
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void unmapIndexBufferData()
{
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

AttributeBindingsHandle createAttributeBindings()
{
	GLuint handle;
	glGenVertexArrays(1, &handle);
	return (AttributeBindingsHandle)handle;
}

void destroyAttributeBindings(const AttributeBindingsHandle& attributeBindings)
{
	glDeleteVertexArrays(1, (const GLuint*)&attributeBindings);
}

void bindAttributes(const AttributeBindingsHandle& attributeBindings)
{
	glBindVertexArray((GLuint)attributeBindings);
}

void bindAttributes(const span<const AttributeBindingInfo>& attributeBindings)
{
	BufferHandle currentBuffer(0);
	for (const auto& info : attributeBindings)
	{
		if (info.buffer != currentBuffer)
			glBindBuffer(GL_ARRAY_BUFFER, (GLuint)info.buffer);
		glEnableVertexAttribArray((GLuint)info.attribute);
		const AttributeTypeInfo& attrTypeInfo = s_attributeTypeInfos[(int)info.type];
		glVertexAttribPointer((GLuint)info.attribute, (GLint)info.count, attrTypeInfo.type, 
				attrTypeInfo.normalized, (GLsizei)info.stride, (void*)(uintptr_t)info.offset);
	}
}

void storeAttributeBindings(const AttributeBindingsHandle& handle, const span<const AttributeBindingInfo>& attributeBindings)
{
	glBindVertexArray((GLuint)handle);
	bindAttributes(attributeBindings);
	bindAttributes(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void bindProgram(not_null<const Program*> program)
{
	glUseProgram(program->programHandle);
}

void setUniform(UniformHandle handle, const mat4& value)
{
	glUniformMatrix4fv(handle, 1, GL_FALSE, &value[0][0]);
}

void setUniform(UniformHandle handle, const TextureChannel& value)
{
    glUniform1i(handle, value);
}


////////////////////////////////////////////////////////////////////////////////////////////////////

TextureHandle create2dTexture(uint width, uint height, not_null<const void*> data)
{
	GLuint texId;	
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, data);

	return TextureHandle(texId);
}

void destroyTexture(const TextureHandle& texture)
{
	glDeleteTextures(1, (const GLuint*)&texture);
}

void bind2dTexture(const TextureChannel& channel, const TextureHandle& texture)
{
    glActiveTexture(channel);	
	glBindTexture(GL_TEXTURE_2D, (GLuint)texture);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void swapWindow(not_null<Window*> window)
{
	SDL_GL_SwapWindow(window->sdlWindow);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void drawIndexed(const PrimitiveType& primitiveType, uint primitiveCount, const IndexType& indexType, uint indexOffset)
{
	static const GLenum ptypes[] = { GL_TRIANGLES };
	static const GLenum pcount[] = { 3 };
	static const GLenum itypes[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT };

	const GLsizei count = primitiveCount * pcount[(int)primitiveType];
	glDrawElements(ptypes[(int)primitiveType], count, itypes[(int)indexType], (const GLvoid*)(uintptr_t)indexOffset);
}

void setViewport(const Rect2i& screenRect)
{
	glViewport(screenRect.p1.x, screenRect.p1.y, screenRect.width(), screenRect.height());
}

void setBlendMode(const BlendMode& blendMode)
{
	switch (blendMode)
	{
		case BlendMode::AlphaBlend:
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case BlendMode::Disabled:
			glDisable(GL_BLEND);
	}
}

void setDepthTestMode(const DepthTestMode& depthTestMode)
{
	if (depthTestMode == DepthTestMode::Disabled)
		glDisable(GL_DEPTH_TEST);
	else
		glEnable(GL_DEPTH_TEST);
}

void setCullMode(const CullMode& cullMode)
{
	if (cullMode == CullMode::Disabled)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE);
}

void setClipMode(const ClipMode& clipMode)
{
	if (clipMode == ClipMode::Disabled)
		glDisable(GL_SCISSOR_TEST);
	else
		glEnable(GL_SCISSOR_TEST);
}
	
void setClipArea(const Rect2i& screenRect)
{
	glScissor(screenRect.p1.x, screenRect.p1.y, screenRect.width(), screenRect.height());
}

void setClearColor(const Color32& color)
{
	glClearColor(color.x, color.y, color.z, color.a);
}

void clearFrameBuffer()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

} // namespace graphics

} // jcpe
