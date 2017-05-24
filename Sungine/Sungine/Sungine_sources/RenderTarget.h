#pragma once

#include <glm/common.hpp>
#include <memory>

#include "FrameBuffer.h"
class Mesh;
class MaterialBlit;
class Texture;
class IRenderTargetWithReflections;
//
//class IRenderTarget
//{
//public:
//	virtual const glm::vec2& getSize() const = 0;
//	virtual void setSize(const glm::vec2& size) = 0;
//
//	virtual void bindFramebuffer() = 0;
//	virtual void unbindFramebuffer() = 0;
//	virtual const GlHelper::Framebuffer& getFrameBuffer() const = 0;
//	virtual GLuint getFinalFrame() const = 0;
//
//	virtual IRenderTargetWithReflections* getAsReflectionRenderTargetIfPossible() { return nullptr; }
//};
//
//class IRenderTargetWithReflections : public IRenderTarget
//{
//public:
//	virtual void bindReflectionFramebuffer() = 0;
//	virtual void unbindReflectionFramebuffer() = 0;
//	virtual const GlHelper::Framebuffer& getReflectionFrameBuffer() const = 0;
//	virtual GLuint getFinalReflectionFrame() const = 0;
//};
//
//class RenderTarget : public IRenderTarget
//{
//protected:
//	glm::vec2 m_size;
//
//	GlHelper::Framebuffer m_frameBuffer;
//	Texture m_texture;
//
//public:
//	RenderTarget(float width = 400, float height = 400);
//
//	const glm::vec2& getSize() const override;
//	void setSize(const glm::vec2& size) override;
//
//	virtual void bindFramebuffer() override;
//	virtual void unbindFramebuffer() override;
//	virtual const GlHelper::Framebuffer& getFrameBuffer() const override;
//	virtual GLuint getFinalFrame() const override;
//
//protected:
//	virtual void onResize();
//};
//
//class RenderTargetWithDepth final : public RenderTarget
//{
//private:
//	GlHelper::Renderbuffer m_depthBuffer;
//
//public:
//	RenderTargetWithDepth(float width = 400, float height = 400);
//
//protected:
//	void onResize() override;
//};
//
//class RenderTargetWithStencil final : public RenderTarget
//{
//private:
//	GlHelper::Renderbuffer m_stencilBuffer;
//
//public:
//	RenderTargetWithStencil(float width = 400, float height = 400);
//
//protected:
//	void onResize() override;
//};
//
//class ViewportRenderTarget final : public RenderTarget, public IRenderTargetWithReflections
//{
//protected:
//	// Reflection buffer
//	GlHelper::Framebuffer m_reflectionFrameBuffer;
//	Texture m_reflectionTexture;
//	GlHelper::Renderbuffer m_reflectionStencilBuffer;
//
//	// Final buffer
//	// Inherited values +
//	GlHelper::Renderbuffer m_depthBuffer;
//
//public:
//	ViewportRenderTarget(float width = 400, float height = 400);
//
//	void bindReflectionFramebuffer() override;
//	void unbindReflectionFramebuffer()override;
//	const GlHelper::Framebuffer& getReflectionFrameBuffer() const override;
//	GLuint getFinalReflectionFrame() const override;
//
//	virtual IRenderTargetWithReflections* getAsReflectionRenderTargetIfPossible() override;
//
//protected:
//	void onResize() override;
//};

////////////////////////////

class RenderTargetLayer
{
protected:
	std::string m_name;
	glm::vec2 m_size;

public:
	RenderTargetLayer(const std::string& name)
		: m_name(name)
	{}

	const std::string& getName() const { return m_name; }
	const glm::vec2& getSize() const { return m_size; }

	virtual void bindFramebuffer(GLenum bindingStatus = GL_FRAMEBUFFER) const = 0;
	virtual void unbindFramebuffer(GLenum bindingStatus = GL_FRAMEBUFFER) const = 0;
	virtual const GlHelper::Framebuffer& getFrameBuffer() const = 0;
	virtual GLuint getFinalFrame() const = 0;
	virtual void onResize(const glm::vec2& newSize) = 0;
};

class ReflectionRTL final : public RenderTargetLayer
{
private:
	// Reflection buffer
	GlHelper::Framebuffer m_reflectionFrameBuffer;
	Texture m_reflectionTexture;
	GlHelper::Renderbuffer m_reflectionStencilBuffer;

public:
	ReflectionRTL();
	void bindFramebuffer(GLenum bindingStatus = GL_FRAMEBUFFER) const override;
	void unbindFramebuffer(GLenum bindingStatus = GL_FRAMEBUFFER) const override;
	const GlHelper::Framebuffer& getFrameBuffer() const override;
	GLuint getFinalFrame() const override;
	virtual void onResize(const glm::vec2& newSize) override;
};

class ColorAndDepthRTL : public RenderTargetLayer
{
private:
	// Reflection buffer
	GlHelper::Framebuffer m_frameBuffer;
	Texture m_texture;
	GlHelper::Renderbuffer m_depthBuffer;

public:
	ColorAndDepthRTL();
	void bindFramebuffer(GLenum bindingStatus = GL_FRAMEBUFFER) const override;
	void unbindFramebuffer(GLenum bindingStatus = GL_FRAMEBUFFER) const override;
	const GlHelper::Framebuffer& getFrameBuffer() const override;
	GLuint getFinalFrame() const override;
	virtual void onResize(const glm::vec2& newSize) override;
};

class RenderTarget
{
protected:
	glm::vec2 m_size;

	std::vector<std::shared_ptr<RenderTargetLayer>> m_layers;

public:
	RenderTarget(std::shared_ptr<RenderTargetLayer> defaultLayer, float width = 400, float height = 400);
	RenderTarget(std::shared_ptr<RenderTargetLayer> firstLayer, std::shared_ptr<RenderTargetLayer> secondLayer, float width = 400, float height = 400);

	const glm::vec2& getSize() const;
	void setSize(const glm::vec2& size);
	
	bool hasLayer(const std::string& layerName);
	bool getLayerNameFromIndex(int idx, std::string & outLayerName) const;
	void bindFramebuffer(int layerIdx = 0);
	void unbindFramebuffer(int layerIdx = 0);
	bool getFrameBuffer(int layerIdx, GlHelper::Framebuffer& outFrameBuffer) const;
	GLuint getFinalFrame(int layerIdx = 0) const;

	void addLayer(std::shared_ptr<RenderTargetLayer> newLayer);
	bool removeLayer(int layerIndex);
	RenderTargetLayer* getLayer(int layerIndex);
	RenderTargetLayer* getLayer(const std::string& layerName);

protected:
	virtual void onResize();
};