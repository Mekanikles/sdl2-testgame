#include "IMGui.h"

#include "Core.h"

#define NK_PRIVATE
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_ASSERT(expr) ASSERT(expr)
#include "nuklear.h"

#include "ColorDefines.h"
#include "Graphics.h"
#include "Profiler.h"

// TODO: Get rid of
#include "SDL.h"

namespace jcpe
{

const auto kProfilerCategoryIMGui = Profiler::CategoryInfo { "IMGui", Color::kBrightPink };

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace imGui
{
	struct NkVertex
	{
		float pos[2];
		float uv[2];
		nk_byte color[4];
	};

	// Limited to 2^16 atm since index is forced uint16
	static const uint MAX_VERTEX_COUNT = 0xFFFF;
	static const uint MAX_INDEX_COUNT =  MAX_VERTEX_COUNT * 3;

	static const char* s_vertexSource = R"(
		#version 150
		uniform mat4 mvp;
		in vec2 pos;
		in vec2 uv;
		in vec4 color;
		out vec2 fragUv;
		out vec4 fragColor;
		void main()
		{
			fragUv = uv;
			fragColor = color;
			gl_Position = mvp * vec4(pos.xy, 1.0, 1.0);
		}
		)";

	static const char* s_fragmentSource = R"(
		#version 150
		precision mediump float;
		uniform sampler2D tex;
		in vec2 fragUv;
		in vec4 fragColor;
		out vec4 outColor;
		void main()
		{
			outColor = fragColor * texture(tex, fragUv.st);
		}
		)";

	static void nk_clipbard_paste(nk_handle usr, nk_text_edit* edit)
	{
		const char* text = SDL_GetClipboardText();
		if (text)
			nk_textedit_paste(edit, text, nk_strlen(text));
		(void)usr;
	}

	static void nk_clipbard_copy(nk_handle usr, const char* text, int len)
	{
		if (!len)
			return;

		// Replace with scratch alloc
		string str;
		str = text;
		SDL_SetClipboardText(str.c_str());
	}

	// Stupid nk adds 0.5 to all coords
	static struct nk_rect toNkRect(const Rect2& rect)
	{
		return nk_rect(rect.x - 0.5f, rect.y - 0.5f, rect.width(), rect.height());
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

struct IMGuiImpl
{
	struct ShaderInfo
	{
		owned_ptr<Graphics::Program> program;
		Graphics::UniformHandle mvpUniform;
		Graphics::UniformHandle texUniform;
		Graphics::AttributeHandle posAttribute;
		Graphics::AttributeHandle uvAttribute;
		Graphics::AttributeHandle colorAttribute;
	};

	struct Context 
	{
		ShaderInfo shaderInfo;
		nk_context nk;
		nk_buffer nkCommands;
		nk_draw_null_texture nkNullTexture;
		nk_font_atlas nkFontAtlas;
		Graphics::TextureHandle fontTexture;
		Graphics::AttributeBindingsHandle attributeBindings;
		Graphics::BufferHandle vertexBuffer;
		Graphics::BufferHandle indexBuffer;
	};

	void setupStyle(not_null<nk_context*> ctx)
	{
    	auto& style = ctx->style;
    	style.window.padding = nk_vec2(0, 0);
    	style.window.group_padding = nk_vec2(0, 0);
		style.window.background = nk_rgba(0,0,0,0);
		style.window.fixed_background = nk_style_item_color(nk_rgba(0,0,0,0));
		style.window.border_color = nk_rgb(255,165,0);
		style.window.combo_border_color = nk_rgb(255,165,0);
		style.window.contextual_border_color = nk_rgb(255,165,0);
		style.window.menu_border_color = nk_rgb(255,165,0);
		style.window.group_border_color = nk_rgb(255,165,0);
		style.window.tooltip_border_color = nk_rgb(255,165,0);
		style.window.scrollbar_size = nk_vec2(16,16);
		style.window.border_color = nk_rgba(0,0,0,0);
		style.window.border = 1;

		style.button.normal = nk_style_item_color(nk_rgba(0,0,0,0));
		style.button.hover = nk_style_item_color(nk_rgb(255,165,0));
		style.button.active = nk_style_item_color(nk_rgb(220,10,0));
		style.button.border_color = nk_rgb(255,165,0);
		style.button.text_background = nk_rgb(0,0,0);
		style.button.text_normal = nk_rgb(255,165,0);
		style.button.text_hover = nk_rgb(28,48,62);
		style.button.text_active = nk_rgb(28,48,62);
	}

	IMGuiImpl()
	{
		using namespace Graphics;	
		Context& ctx = this->context;

		// Setup nk
		nk_init_default(&ctx.nk, nullptr);
		nk_buffer_init_default(&ctx.nkCommands);
		ctx.nk.clip.copy = imGui::nk_clipbard_copy;
		ctx.nk.clip.paste = imGui::nk_clipbard_paste;
		ctx.nk.clip.userdata = nk_handle_ptr(nullptr);

		// Setup 2d texture shader
		ShaderInfo& shaderInfo = ctx.shaderInfo;
		ProgramCreationParams programParams = { imGui::s_vertexSource, imGui::s_fragmentSource };
		shaderInfo.program = createProgram(programParams);
		shaderInfo.mvpUniform = fetchUniformHandle(shaderInfo.program, "mvp");
		shaderInfo.texUniform = fetchUniformHandle(shaderInfo.program, "tex");
		shaderInfo.posAttribute = fetchAttributeHandle(shaderInfo.program, "pos");
		shaderInfo.uvAttribute = fetchAttributeHandle(shaderInfo.program, "uv");
		shaderInfo.colorAttribute = fetchAttributeHandle(shaderInfo.program, "color");

		// Setup shader attributes
		ctx.vertexBuffer = createBuffer();
		ctx.indexBuffer = createBuffer();		
		const uint vertexSize = sizeof(imGui::NkVertex);
		const uint posOffset = offsetof(imGui::NkVertex, pos);
		const uint uvOffset = offsetof(imGui::NkVertex, uv);
		const uint colorOffset = offsetof(imGui::NkVertex, color);
		ctx.attributeBindings = createAttributeBindings();

		const AttributeBindingInfo attributeInfos[] = {
				{ shaderInfo.posAttribute, ctx.vertexBuffer, AttributeType::Float, 2, vertexSize, posOffset },
				{ shaderInfo.uvAttribute, ctx.vertexBuffer, AttributeType::Float, 2, vertexSize, uvOffset },
				{ shaderInfo.colorAttribute, ctx.vertexBuffer, AttributeType::NormalizedUInt8, 4, vertexSize, colorOffset }};
		storeAttributeBindings(ctx.attributeBindings, attributeInfos);

		// Setup font
		{
			nk_font_atlas_init_default(&ctx.nkFontAtlas);
			nk_font_atlas_begin(&ctx.nkFontAtlas);
		
			int width;
			int height;
			const void* fontImage = nk_font_atlas_bake(&ctx.nkFontAtlas, &width, &height, NK_FONT_ATLAS_RGBA32);

			ctx.fontTexture = create2dTexture(width, height, fontImage);

    		nk_font_atlas_end(&ctx.nkFontAtlas, nk_handle_id((int)ctx.fontTexture), &ctx.nkNullTexture);
			if (ctx.nkFontAtlas.default_font)
				nk_style_set_font(&ctx.nk, &ctx.nkFontAtlas.default_font->handle);
		}

		setupStyle(&ctx.nk);
	}

	~IMGuiImpl()
	{
		using namespace Graphics;	
		Context& ctx = this->context;
		nk_font_atlas_clear(&ctx.nkFontAtlas);
		nk_free(&ctx.nk);
		nk_buffer_free(&ctx.nkCommands);
		destroyProgram(std::move(ctx.shaderInfo.program));
		destroyBuffer(ctx.vertexBuffer);
		destroyBuffer(ctx.indexBuffer);
		destroyAttributeBindings(ctx.attributeBindings);
		destroyTexture(ctx.fontTexture);
	}

	Context context;
	nk_command_buffer* canvas = nullptr;
	vec2 lastCanvasSize;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

IMGui::IMGui()
	: m_impl(make_unique<IMGuiImpl>())
{


}

IMGui::~IMGui()
{


}

void IMGui::beginFrame(const vec2& canvasSize)
{
	PROFILER_SCOPE("IMGuiBegin", &kProfilerCategoryIMGui);

	IMGuiImpl::Context& ctx = m_impl->context;
	nk_begin(&ctx.nk, "imgui", imGui::toNkRect(Rect2(Point2(0, 0), canvasSize)), NK_WINDOW_NO_SCROLLBAR);
	m_impl->canvas = nk_window_get_canvas(&ctx.nk);
	m_impl->lastCanvasSize = canvasSize;
}
	
void IMGui::endFrame()
{
	PROFILER_SCOPE("IMGuiEnd", &kProfilerCategoryIMGui);

	IMGuiImpl::Context& ctx = m_impl->context;
	nk_end(&ctx.nk);
	m_impl->canvas = nullptr;
}

void IMGui::filledRect(const Rect2& rect, const Color32& color)
{
	nk_command_buffer* canvas = m_impl->canvas;
	FATAL_ASSERT(canvas);
	const auto c = color.bytes();
	nk_fill_rect(canvas, imGui::toNkRect(rect), 
			0, nk_rgba(c.x, c.y, c.z, c.w));
}

void IMGui::filledCircle(const Rect2& rect, const Color32& color)
{
	nk_command_buffer* canvas = m_impl->canvas;
	FATAL_ASSERT(canvas);
	auto c = color.bytes();
	nk_fill_circle(canvas, imGui::toNkRect(rect), 
			nk_rgba(c.x, c.y, c.z, c.w));
}

void IMGui::strokedCircle(const Rect2& rect, float lineWidth, const Color32& color)
{
	nk_command_buffer* canvas = m_impl->canvas;
	FATAL_ASSERT(canvas);
	const auto c = color.bytes();
	nk_stroke_circle(canvas, imGui::toNkRect(rect), 
			lineWidth, nk_rgba(c.x, c.y, c.z, c.w));
}

void IMGui::text(const Rect2& rect, const string& text, const Color32& color)
{
	nk_command_buffer* canvas = m_impl->canvas;
	const auto c = color.bytes();
	const auto style = &m_impl->context.nk.style;

    struct nk_text textInfo;
    textInfo.padding.x = style->text.padding.x;
   	textInfo.padding.y = style->text.padding.y;
    textInfo.background = nk_rgba(0, 0, 0, 0);
    textInfo.text = nk_rgba(c.x, c.y, c.z, c.w);

	nk_widget_text(canvas, imGui::toNkRect(rect), text.c_str(), 
			text.length(), &textInfo, NK_TEXT_CENTERED, style->font);

	//nk_draw_text(canvas, imGui::toNkRect(rect), text.c_str(), 
	//	text.length(), style->font, nk_rgba(0, 0, 0, 0), nk_rgba(c.x, c.y, c.z, c.w));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGui::render(not_null<const Graphics::Window*> window)
{
	PROFILER_SCOPE("IMGuiRender", &kProfilerCategoryIMGui);

	using namespace Graphics;	

	// TODO: Make const? Move convert to other function render/dispatch?
	IMGuiImpl::Context& ctx = m_impl->context;
	const vec2& canvasSize = m_impl->lastCanvasSize;

	const mat4 projMatrix = math::ortho(0.0f, canvasSize.x, canvasSize.y, 0.0f);

	const vec2i windowSize = Graphics::getWindowSize(window);
	const vec2 scale = vec2(windowSize.x / canvasSize.x, windowSize.y / canvasSize.y);

	// Set up frame
	setBlendMode(BlendMode::AlphaBlend);
	setDepthTestMode(DepthTestMode::Disabled);
	setCullMode(CullMode::Disabled);
	setClipMode(ClipMode::Enabled);
 	
	bindProgram(ctx.shaderInfo.program);
	setUniform(ctx.shaderInfo.mvpUniform, projMatrix);
	setUniform(ctx.shaderInfo.texUniform, TextureChannel(0));

	// Write data to buffers
	{
		const auto vBufferSize = imGui::MAX_VERTEX_COUNT * sizeof(imGui::NkVertex);
		const auto iBufferSize = imGui::MAX_INDEX_COUNT * sizeof(uint16);	
		const not_null<imGui::NkVertex*> vertices = (imGui::NkVertex*)createAndMapVertexBufferData(ctx.vertexBuffer, vBufferSize);
		const not_null<uint16*> indices = (uint16*)createAndMapIndexBufferData(ctx.indexBuffer, iBufferSize);
	
		/* fill convert configuration */
		static const nk_draw_vertex_layout_element vertex_layout[] = {
			{NK_VERTEX_POSITION, NK_FORMAT_FLOAT, offsetof(imGui::NkVertex, pos)},
			{NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, offsetof(imGui::NkVertex, uv)},
			{NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, offsetof(imGui::NkVertex, color)},
			{NK_VERTEX_LAYOUT_END}};

		nk_convert_config config;
		NK_MEMSET(&config, 0, sizeof(config));
		config.vertex_layout = vertex_layout;
		config.vertex_size = sizeof(imGui::NkVertex);
		config.vertex_alignment = NK_ALIGNOF(imGui::NkVertex);
		config.null = ctx.nkNullTexture;
		config.circle_segment_count = 32;
		config.curve_segment_count = 32;
		config.arc_segment_count = 32;
		config.global_alpha = 1.0f;
		config.shape_AA = NK_ANTI_ALIASING_OFF;
		config.line_AA = NK_ANTI_ALIASING_OFF;

		/* setup buffers to load vertices and elements */
		nk_buffer vbuf, ibuf;
		nk_buffer_init_fixed(&vbuf, vertices, (nk_size)vBufferSize);
		nk_buffer_init_fixed(&ibuf, indices, (nk_size)iBufferSize);
		nk_convert(&ctx.nk, &ctx.nkCommands, &vbuf, &ibuf, &config);
		
		unmapVertexBufferData();
		unmapIndexBufferData();
	}

	// Draw
	{
		bindAttributes(ctx.attributeBindings);

		const nk_draw_index* offset = 0;
		for(const nk_draw_command* cmd = nk__draw_begin(&ctx.nk, &ctx.nkCommands); 
				cmd != nullptr; 
				cmd = nk__draw_next(cmd, &ctx.nkCommands, &ctx.nk))
		{
			if (!cmd->elem_count) 
				continue;
			bind2dTexture(TextureChannel(0), (TextureHandle)cmd->texture.id);
			const Rect2i clipRect = Rect2i(Point2i(cmd->clip_rect.x * scale.x,
					(canvasSize.y - (cmd->clip_rect.y + cmd->clip_rect.h)) * scale.y),
					vec2i(cmd->clip_rect.w * scale.x, cmd->clip_rect.h * scale.y));
			setClipArea(clipRect);
			drawIndexed(PrimitiveType::Triangles, cmd->elem_count / 3, IndexType::UInt16, (uint)(size_t)offset);
			offset += cmd->elem_count;
		}
	}

	// Clear command buffers etc
	nk_clear(&ctx.nk);	
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}