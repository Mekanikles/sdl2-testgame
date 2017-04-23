
#include "Graphics.h"

#include "SDL.h"
#include "SDL_assert.h"

#include "Core.h"

using namespace jcpe;

void render()
{
	Graphics::setClearColor(ColorRGBA(1.0f, 0.5f, 0.25f, 1.0f));
	Graphics::clearFrameBuffer();
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
            case SDL_WINDOWEVENT:
            {
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
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

	SCOPE_EXIT( SDL_Quit(); );

	Graphics::WindowCreationParams windowParams = { 640, 480 };
	owned_ptr<Graphics::Window> window = Graphics::createWindow(windowParams);
	if (!window)
		return 1;

	SCOPE_EXIT( Graphics::destroyWindow(std::move(window)); );

    owned_ptr<Graphics::Context> context = Graphics::createContext(window);
	if (!context)
		return 1;

	SCOPE_EXIT( Graphics::destroyContext(std::move(context)); );

	// Shader sources
	const char* vertexSource = R"(
        #version 100
		attribute vec4 position;
		void main()
		{
			gl_Position = vec4(position.xyz, 1.0);
		}
		)";

	const char* fragmentSource = R"(
        #version 100
		precision mediump float;
		void main()
		{
			gl_FragColor = vec4 (1.0, 1.0, 1.0, 1.0 );
		}
		)";

	Graphics::ProgramCreationParams programParams = { vertexSource, fragmentSource };
	owned_ptr<Graphics::Program> program = Graphics::createProgram(programParams);
	if (!program)
		return 1;

	SCOPE_EXIT( Graphics::destroyProgram(std::move(program)); );

	while (!mainloop())
	{	
		Graphics::swapBuffers(window);
		SDL_Delay(16);
	}

    return 0;
}
