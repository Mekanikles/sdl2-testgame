
#if defined(__IPHONEOS__) || defined(__ANDROID__)
	#define SUPPORTS_OPENGLES
#endif
  
#ifdef SUPPORTS_OPENGLES
	#include "SDL_opengles2.h"
#else
	#include "SDL_opengl.h"
#endif

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

namespace Renderer
{

void set2dProjection()
{
	const float aspectAdjust = 16.0f/9.0f;
#ifdef SUPPORTS_OPENGLES // GLES supports fixed point, but GL does not
	glOrthof(-2.0, 2.0, -2.0 * aspectAdjust, 2.0 * aspectAdjust, -20.0, 20.0);
#else
	glOrtho(-2.0, 2.0, -2.0 * aspectAdjust, 2.0 * aspectAdjust, -20.0, 20.0);
#endif
}

void initRendere



}