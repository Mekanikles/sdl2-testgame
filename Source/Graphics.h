#pragma once

#include "Core.h"

namespace jcpe
{

namespace Graphics
{
	struct Window;
	struct Context; // TODO: Make this into an interface for rendering primitives? (RenderContext)
	struct Program;

	using UniformHandle = TypeWrapper<uint>;
	using AttributeHandle = TypeWrapper<uint>;
	using BufferHandle = TypeWrapper<uint>;
	using AttributeBindingsHandle = TypeWrapper<uint>;		
	using TextureHandle = TypeWrapper<uint>;
	using TextureChannel = TypeWrapper<uint>;
	using BufferAttributeBinding = TypeWrapper<uint>;

	enum class BlendMode
	{
		Disabled,
		AlphaBlend
	};

	enum class CullMode
	{
		Disabled,
		Enabled	
	};

	enum class DepthTestMode
	{
		Disabled,
		Enabled
	};

	enum class ClipMode
	{
		Disabled,
		Enabled
	};

	enum class AttributeType
	{
		Float = 0,
		NormalizedUInt8
	};

	enum class PrimitiveType
	{
		Triangles = 0
	};

	enum class IndexType
	{
		UInt8 = 0,
		UInt16,
		UInt32
	};

	struct AttributeBindingInfo
	{
		AttributeHandle attribute;
		BufferHandle buffer;
		AttributeType type;
		uint count;
		uint stride;
		uint offset;
	};

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

	vec2i getWindowSize(not_null<const Window*> window);
	vec2 getWindowCanvasSize(not_null<const Window*> window);
	/*
		Context creation/destruction
	*/

	owned_ptr<Context> createContext(not_null<Window*> window);
	void destroyContext(owned_ptr<Context> context);

	/*
		Shader program creation/destruction
	*/

	struct ProgramCreationParams
	{
		not_null<const char*> vertexSource;
		not_null<const char*> fragmentSource;
	};

	owned_ptr<Program> createProgram(const ProgramCreationParams& params);
	void destroyProgram(owned_ptr<Program> program);

	UniformHandle fetchUniformHandle(not_null<const Program*> program, const string& name);
	AttributeHandle fetchAttributeHandle(not_null<const Program*> program, const string& name);

	/*
		Operations
	*/

	BufferHandle createBuffer();
	void destroyBuffer(const BufferHandle& buffer);

	// TODO: Add buffer characteristics
	void* createAndMapVertexBufferData(const BufferHandle& buffer, const uint capacity);
	void* createAndMapIndexBufferData(const BufferHandle& buffer, const uint capacity);
	void unmapVertexBufferData();
	void unmapIndexBufferData();

	AttributeBindingsHandle createAttributeBindings();
	void destroyAttributeBindings(const AttributeBindingsHandle& attributeBindings);	
	void bindAttributes(const AttributeBindingsHandle& attributeBindings);
	void bindAttributes(const span<const AttributeBindingInfo>& attributeBindings);
	void storeAttributeBindings(const AttributeBindingsHandle& handle, const span<const AttributeBindingInfo>& attributeBindings);

	void bindProgram(not_null<const Program*> program);


	void setUniform(UniformHandle handle, const mat4& value);
	void setUniform(UniformHandle handle, const TextureChannel& value);

	TextureHandle create2dTexture(uint width, uint height, not_null<const void*> data);
	void destroyTexture(const TextureHandle& texture);
	void bind2dTexture(const TextureChannel& channel, const TextureHandle& texture);

	/*
		Frame functions
	*/

	void swapWindow(not_null<Window*> window);

	/*
		Rendering primitive functions
	*/

	void drawIndexed(const PrimitiveType& primitiveType, uint primitiveCount, const IndexType& indexType, uint indexOffset);

	void setViewport(const Rect2i& screenRect);
	void setBlendMode(const BlendMode& blendMode);
	void setDepthTestMode(const DepthTestMode& depthTestMode);
	void setCullMode(const CullMode& cullMode);
	void setClipMode(const ClipMode& clipMode);
	void setClipArea(const Rect2i& screenRect);

	void setClearColor(const Color32& color);
	void clearFrameBuffer();

} // namespace graphics

}
