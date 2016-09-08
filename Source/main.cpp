
#include "Graphics.h"

#include "SDL.h"
#include <stdio.h>

#include "Core.h"

using namespace jcpe;

void render()
{
	jcpe::graphics::setClearColor(ColorRGBA(1.0f, 0.5f, 0.25f, 1.0f));
	jcpe::graphics::clearFrameBuffer();
}

bool mainloop()
{
	bool done = false;

	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_KEYDOWN:
			{
				if (event.key.state == SDL_PRESSED && event.key.keysym.sym == SDLK_ESCAPE)
					done = true;
				break;
			}
		}
	}

	render();

	return done;
}

int main(int argc, char* argv[]) 
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		LOG("Failed to initialize SDL: " <<  SDL_GetError());
		return 1;
	}

	AT_SCOPE_EXIT( SDL_Quit(); );

	jcpe::graphics::WindowCreationParams windowParams = { 640, 480 };
	owned_ptr<jcpe::graphics::Window> window = jcpe::graphics::createWindow(windowParams);
	if (!window)
		return 1;

	AT_SCOPE_EXIT( jcpe::graphics::destroyWindow(std::move(window)); );

    owned_ptr<jcpe::graphics::Context> context = jcpe::graphics::createContext(window);
	if (!context)
		return 1;

	AT_SCOPE_EXIT( jcpe::graphics::destroyContext(std::move(context)); );

	// Shader sources
	const char* vertexSource = R"(
		attribute vec4 position;
		void main()
		{
			gl_Position = vec4(position.xyz, 1.0);
		}
		)";

	const char* fragmentSource = R"(
		precision mediump float;
		void main()
		{
			gl_FragColor = vec4 (1.0, 1.0, 1.0, 1.0 );
		}
		)";

	jcpe::graphics::ProgramCreationParams programParams = { vertexSource, fragmentSource };
	owned_ptr<jcpe::graphics::Program> program = jcpe::graphics::createProgram(programParams);
	if (!program)
		return 1;

	AT_SCOPE_EXIT( jcpe::graphics::destroyProgram(std::move(program)); );

	while (!mainloop())
	{	
		jcpe::graphics::swapBuffers(window);
		SDL_Delay(16);
	}

    return 0;
}
