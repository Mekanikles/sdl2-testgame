
#include <thread>
#include <cxxabi.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include <execinfo.h>
#include "Graphics.h"

#include "SDL.h"
#include "SDL_assert.h"

#include "Core.h"

#include "IMGui.h"

namespace jcpe
{
	// TODO: Move into engine code
	unsigned int s_frame = 0;
}

using namespace jcpe;

static owned_ptr<Graphics::Window> s_window;
static unique_ptr<IMGui> s_imGui;


const float s_frameDelayMs = 16;

const float s_ballGrowth = 0.1f;
const float s_ballHitShrink = 0.4f;
const float s_ballMinRadius = 5.f;
const float s_ballStartRadius = 10.f;
bool s_ballSetup = false;
const int s_ballCountX = 20;
const int s_ballCountY = 20;
struct Ball
{
	vec2 pos;
	vec2 vel;
	float radius;
	float expandRate;
	Color32 color;
};

vector<Ball> s_balls;

void setupBalls(const vec2& canvasSize)
{
	s_balls.reserve(s_ballCountX * s_ballCountY);

	const vec2 step = vec2((canvasSize.x / (float)s_ballCountX), 
			(canvasSize.y / (float)s_ballCountY));

	for (int y = 0; y < s_ballCountY; ++y)
	{
		for (int x = 0; x < s_ballCountX; ++x)
		{
			const vec2 mid = vec2(step.x * (x + 0.5f), step.y * (y + 0.5f));
			const vec2 vel = vec2((x + 0.5f) / (float)s_ballCountX, (y + 0.5f) / (float)s_ballCountY);

			Ball b;
			b.pos = mid;
			b.vel = vel * 0.1f;
			b.radius = s_ballStartRadius;
			b.expandRate = 0;
			b.color = Color32(vel.x, (vel.x + vel.y) * 0.5f, vel.y, 1.0f);		
			s_balls.push_back(b);
		}
	}

	s_ballSetup = true;
}

void simulateBalls(const vec2& canvasSize)
{
	// Move
	for (auto& b : s_balls)
	{
		const vec2 a = vec2(0, 0.5f);
		b.pos += b.vel + a * a;
		b.vel += a;
	}

	// Collide
	for (int i = 0; i < 5; ++i)
	{	
	const int ballCount = s_balls.size();
	for (int b1i = 0; b1i < ballCount; ++b1i)	
	{
		auto& b = s_balls[b1i];

		// Balls
		vec2 impulse = vec2(0, 0);
		for (int b2i = b1i + 1; b2i < ballCount; ++b2i)	
		{
			auto& b2 = s_balls[b2i];

			const vec2 v = b2.pos - b.pos;
			const float ds = math::distance2(b2.pos, b.pos);
			if (ds == 0.0f)
				continue;
			if (ds < ((b.radius + b2.radius) * (b.radius + b2.radius)))
			{
				const float len = sqrt(ds);
				const float overlap = (b.radius + b2.radius) - len;
				const vec2 n = (v / len);
				const vec2 sep = -overlap * 0.5f * n;
				impulse += sep;

				b.pos += sep;
				b2.pos -= sep;

				const vec2 rvel = b.vel - b2.vel;
				const float nvel = math::dot(rvel, n);
				if (nvel > 0.01)
				{
					b.vel -= nvel * n * 0.95f;
					b2.vel += nvel * n * 0.95f;
				}
			}
		}

		// Walls
		vec2 adjust = vec2(0,0);
		if (b.pos.x < b.radius)
			adjust.x = b.radius - b.pos.x;
		if (b.pos.x > canvasSize.x - b.radius)
			adjust.x = (canvasSize.x - b.radius) - b.pos.x;
		if (b.pos.y < b.radius)
			adjust.y = b.radius - b.pos.y;
		if (b.pos.y > canvasSize.y - b.radius)
			adjust.y = (canvasSize.y - b.radius) - b.pos.y;

		b.pos += adjust;
		const vec2 n = math::normalize(adjust);
		const float nvel = math::dot(b.vel, n);
		if (nvel < 0.01)
		{
			b.vel -= nvel * n * 1.9f;
		}		
	}
	}
}

void drawBalls()
{
	for (auto& b : s_balls)
	{
		s_imGui->filledCircle(Rect2(Point2(b.pos.x - b.radius, b.pos.y - b.radius), vec2(b.radius * 2, b.radius * 2)), b.color);
	}
}

void render()
{
	using namespace Graphics;

	setViewport(Rect2i(Point2i(0, 0), Graphics::getWindowSize(s_window)));

	setClearColor(Color32(1.0f, 1.0f, 1.0f, 1.0f));
	clearFrameBuffer();

	s_imGui->render(s_window);
}

bool mainloop()
{
	++s_frame;

	bool done = false;
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT: done = true; break;
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
			default:
			{
			}
		}
	}

	const vec2 canvasize = Graphics::getWindowCanvasSize(s_window);
	s_imGui->beginFrame(vec2(canvasize.x, canvasize.y));

	if (!s_ballSetup)
		setupBalls(canvasize);

	simulateBalls(canvasize);
	drawBalls();

	//s_imGui->filledCircle(Rect2(Point2(50, 50), vec2(25, 25)), Color::red);

	//s_imGui->strokedCircle(Rect2(Point2(100, 100), vec2(25, 25)), 2, Color::green);

	//s_imGui->text(Rect2(Point2(150, 150), vec2(25, 25)), "Testing text", Color::blue);

	s_imGui->endFrame();

	render();

	return done;
}

int run()
{
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");	
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0)
	{
		LOG("Failed to initialize SDL: " <<  SDL_GetError());
		return 1;
	}

	SCOPE_EXIT( SDL_Quit(); );

	Graphics::WindowCreationParams windowParams = { 800, 600 };
	s_window = Graphics::createWindow(windowParams);
	if (!s_window)
		return 1;

	SCOPE_EXIT( Graphics::destroyWindow(std::move(s_window)); );

    owned_ptr<Graphics::Context> context = Graphics::createContext(s_window);
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

	s_imGui = make_unique<IMGui>();

	while (!mainloop())
	{	
		Graphics::swapWindow(s_window);
		SDL_Delay(s_frameDelayMs);
	}

	return 0;
}

void
handler()
{
	void* traceElems[40];
	int traceElemCount = backtrace(traceElems, 40);
	char** stackSyms = backtrace_symbols(traceElems, traceElemCount);

	std::ostringstream os;
	os << "Thread " << std::this_thread::get_id() << " terminated, stack trace:" << std::endl;
	for (int i = 0; i < traceElemCount; ++i)
	{
		os << "    ";

		std::istringstream iss(stackSyms[i]);
    	string word;
    	uint count = 0;
    	while(iss >> word)
		{
			static const int minColumnWidths[] = { 3, 20, 0, 0, 0, 0 };

			int status;
			char* unmangled = abi::__cxa_demangle(word.c_str(), 0, 0, &status);
			if (status == 0)
			{
				word = unmangled;
				delete unmangled;
			}

			if (word.length() < minColumnWidths[count])
				word.append(minColumnWidths[count] - word.length(), ' ');

			os << " " << word;
			++count;
		}

		os << std::endl;
	}
	LOG_RAW(os.str());
	delete stackSyms;

	std::abort();
}   

int main(int argc, char* argv[]) 
{
	std::set_terminate(handler);
	return run();
}
