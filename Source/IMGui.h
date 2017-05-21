#pragma once

#include "Core.h"

namespace jcpe
{

struct IMGuiImpl;
namespace Graphics
{
	struct Window;
}

class IMGui
{
public:
	IMGui();
	~IMGui();

	void beginFrame(const vec2& canvasSize);
	void endFrame();

	void filledRect(const Rect2& rect, const Color32& color);
	void filledCircle(const Rect2& rect, const Color32& color);
	void strokedCircle(const Rect2& rect, float lineWidth, const Color32& color);	
	void text(const Rect2& rect, const string& text, const Color32& color);

	void render(not_null<const Graphics::Window*> window);


private:
	unique_ptr<IMGuiImpl> m_impl;
};

}