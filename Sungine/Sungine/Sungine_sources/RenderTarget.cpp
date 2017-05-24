#include "RenderTarget.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include <algorithm>

//RenderTarget::RenderTarget(float width, float height)
//	: m_size(width, height)
//{
//	// Setup texture
//	GlHelper::makeFloatColorTexture(m_texture, m_size.x, m_size.y);
//	m_texture.initGL();
//	// Setup framebuffer
//	m_frameBuffer.bind();
//		m_frameBuffer.attachTexture(&m_texture, GL_COLOR_ATTACHMENT0);
//	m_frameBuffer.checkIntegrity();
//	m_frameBuffer.unbind();
//}
//
//const glm::vec2 & RenderTarget::getSize() const
//{
//	return m_size;
//}
//
//void RenderTarget::setSize(const glm::vec2 & size)
//{
//	m_size = size;
//	onResize();
//}
//
//void RenderTarget::onResize()
//{
//	m_frameBuffer.bind();
//	m_frameBuffer.detachTexture(GL_COLOR_ATTACHMENT0);
//
//	m_texture.freeGL();
//	GlHelper::makeFloatColorTexture(m_texture, m_size.x, m_size.y);
//	m_texture.initGL();
//
//	m_frameBuffer.attachTexture(&m_texture, GL_COLOR_ATTACHMENT0);
//	m_frameBuffer.checkIntegrity();
//	m_frameBuffer.unbind();
//}
//
//void RenderTarget::bindFramebuffer()
//{
//	m_frameBuffer.bind();
//}
//
//void RenderTarget::unbindFramebuffer()
//{
//	m_frameBuffer.unbind();
//}
//
//const GlHelper::Framebuffer & RenderTarget::getFrameBuffer() const
//{
//	return m_frameBuffer;
//}
//
//GLuint RenderTarget::getFinalFrame() const
//{
//	return m_texture.glId;
//}
//
/////////////////////////////////////////////////
//
//RenderTargetWithDepth::RenderTargetWithDepth(float width, float height)
//	: RenderTarget(width, height)
//	, m_depthBuffer(width, height, GL_DEPTH_COMPONENT24)
//{
//	// Setup texture
//	GlHelper::makeFloatColorTexture(m_texture, m_size.x, m_size.y);
//	m_texture.initGL();
//	// Setup framebuffer
//	m_frameBuffer.bind();
//	m_frameBuffer.attachTexture(&m_texture, GL_COLOR_ATTACHMENT0);
//	m_frameBuffer.attachRenderBuffer(&m_depthBuffer, GL_DEPTH_ATTACHMENT);
//	m_frameBuffer.checkIntegrity();
//	m_frameBuffer.unbind();
//}
//
//void RenderTargetWithDepth::onResize()
//{
//	m_frameBuffer.bind();
//	m_frameBuffer.detachTexture(GL_COLOR_ATTACHMENT0);
//
//	m_texture.freeGL();
//	GlHelper::makeFloatColorTexture(m_texture, m_size.x, m_size.y);
//	m_texture.initGL();
//
//	m_frameBuffer.detachRenderBuffer(GL_DEPTH_ATTACHMENT);
//	m_depthBuffer.resize(m_size.x, m_size.y);
//	m_frameBuffer.attachRenderBuffer(&m_depthBuffer, GL_DEPTH_ATTACHMENT);
//
//	m_frameBuffer.attachTexture(&m_texture, GL_COLOR_ATTACHMENT0);
//	m_frameBuffer.checkIntegrity();
//	m_frameBuffer.unbind();
//}
//
//RenderTargetWithStencil::RenderTargetWithStencil(float width, float height)
//	: RenderTarget(width, height)
//	, m_stencilBuffer(width, height, GL_STENCIL_INDEX8)
//{
//	// Setup texture
//	GlHelper::makeFloatColorTexture(m_texture, m_size.x, m_size.y);
//	m_texture.initGL();
//	// Setup framebuffer
//	m_frameBuffer.bind();
//		m_frameBuffer.attachTexture(&m_texture, GL_COLOR_ATTACHMENT0);
//		m_frameBuffer.attachRenderBuffer(&m_stencilBuffer, GL_STENCIL_ATTACHMENT);
//	m_frameBuffer.checkIntegrity();
//	m_frameBuffer.unbind();
//}
//
//void RenderTargetWithStencil::onResize()
//{
//	m_frameBuffer.bind();
//	m_frameBuffer.detachTexture(GL_COLOR_ATTACHMENT0);
//
//	m_texture.freeGL();
//	GlHelper::makeFloatColorTexture(m_texture, m_size.x, m_size.y);
//	m_texture.initGL();
//
//	m_frameBuffer.detachRenderBuffer(GL_STENCIL_ATTACHMENT);
//	m_stencilBuffer.resize(m_size.x, m_size.y);
//	m_frameBuffer.attachRenderBuffer(&m_stencilBuffer, GL_STENCIL_ATTACHMENT);
//
//	m_frameBuffer.attachTexture(&m_texture, GL_COLOR_ATTACHMENT0);
//	m_frameBuffer.checkIntegrity();
//	m_frameBuffer.unbind();
//}
//
//ViewportRenderTarget::ViewportRenderTarget(float width, float height)
//	: RenderTarget(width, height)
//	, m_depthBuffer(width, height, GL_DEPTH_COMPONENT24)
//	, m_reflectionStencilBuffer(width, height, GL_STENCIL_INDEX8)
//{
//	//////////////
//	// Setup Reflection buffer
//	// Setup texture
//	GlHelper::makeFloatColorTexture(m_reflectionTexture, m_size.x, m_size.y);
//	m_reflectionTexture.initGL();
//	// Setup framebuffer
//	m_reflectionFrameBuffer.bind();
//	m_reflectionFrameBuffer.attachRenderBuffer(&m_reflectionStencilBuffer, GL_STENCIL_ATTACHMENT);
//	m_reflectionFrameBuffer.attachTexture(&m_texture, GL_COLOR_ATTACHMENT0);
//	m_reflectionFrameBuffer.checkIntegrity();
//	m_reflectionFrameBuffer.unbind();
//
//	//////////////
//	// Add Depth to final buffer
//	m_frameBuffer.bind();
//	m_frameBuffer.attachRenderBuffer(&m_depthBuffer, GL_DEPTH_ATTACHMENT);
//	m_frameBuffer.checkIntegrity();
//	m_frameBuffer.unbind();
//}
//
//void ViewportRenderTarget::onResize()
//{
//	m_frameBuffer.bind();
//	m_frameBuffer.detachTexture(GL_COLOR_ATTACHMENT0);
//
//	m_texture.freeGL();
//	GlHelper::makeFloatColorTexture(m_texture, m_size.x, m_size.y);
//	m_texture.initGL();
//
//	m_frameBuffer.detachRenderBuffer(GL_DEPTH_ATTACHMENT);
//	m_depthBuffer.resize(m_size.x, m_size.y);
//	m_frameBuffer.attachRenderBuffer(&m_depthBuffer, GL_DEPTH_ATTACHMENT);
//
//	m_frameBuffer.attachTexture(&m_texture, GL_COLOR_ATTACHMENT0);
//	m_frameBuffer.checkIntegrity();
//	m_frameBuffer.unbind();
//
//	//////////
//
//	m_reflectionFrameBuffer.bind();
//	m_reflectionFrameBuffer.detachTexture(GL_COLOR_ATTACHMENT0);
//
//	m_texture.freeGL();
//	GlHelper::makeFloatColorTexture(m_texture, m_size.x, m_size.y);
//	m_texture.initGL();
//	m_reflectionFrameBuffer.attachTexture(&m_texture, GL_COLOR_ATTACHMENT0);
//
//	m_reflectionFrameBuffer.detachRenderBuffer(GL_STENCIL_ATTACHMENT);
//	m_reflectionStencilBuffer.resize(m_size.x, m_size.y);
//	m_reflectionFrameBuffer.attachRenderBuffer(&m_depthBuffer, GL_STENCIL_ATTACHMENT);
//
//	m_reflectionFrameBuffer.checkIntegrity();
//	m_reflectionFrameBuffer.unbind();
//}
// 
//void ViewportRenderTarget::bindReflectionFramebuffer()
//{
//	m_reflectionFrameBuffer.bind();
//}
//
//void ViewportRenderTarget::unbindReflectionFramebuffer()
//{
//	m_reflectionFrameBuffer.unbind();
//}
//
//const GlHelper::Framebuffer & ViewportRenderTarget::getReflectionFrameBuffer() const
//{
//	return m_reflectionFrameBuffer;
//}
//
//GLuint ViewportRenderTarget::getFinalReflectionFrame() const
//{
//	return m_reflectionTexture.glId;
//}
//
//IRenderTargetWithReflections * ViewportRenderTarget::getAsReflectionRenderTargetIfPossible()
//{
//	return this;
//}

//////////////////////////////////

ReflectionRTL::ReflectionRTL()
	: RenderTargetLayer("Reflection")
	, m_reflectionStencilBuffer(400, 400, GL_STENCIL_INDEX8)
{
	// Setup texture
	GlHelper::makeFloatColorTexture(m_reflectionTexture, m_size.x, m_size.y);
	m_reflectionTexture.initGL();
	// Setup framebuffer
	m_reflectionFrameBuffer.bind();
	m_reflectionFrameBuffer.attachTexture(&m_reflectionTexture, GL_COLOR_ATTACHMENT0);
	m_reflectionFrameBuffer.attachRenderBuffer(&m_reflectionStencilBuffer, GL_STENCIL_ATTACHMENT);
	m_reflectionFrameBuffer.checkIntegrity();
	m_reflectionFrameBuffer.unbind();
}

void ReflectionRTL::bindFramebuffer(GLenum bindingStatus) const
{
	m_reflectionFrameBuffer.bind(bindingStatus);
}

void ReflectionRTL::unbindFramebuffer(GLenum bindingStatus) const
{
	m_reflectionFrameBuffer.unbind(bindingStatus);
}

const GlHelper::Framebuffer & ReflectionRTL::getFrameBuffer() const
{
	return m_reflectionFrameBuffer;
}

GLuint ReflectionRTL::getFinalFrame() const
{
	return m_reflectionTexture.glId;
}

void ReflectionRTL::onResize(const glm::vec2& newSize)
{
	m_size = newSize;

	m_reflectionFrameBuffer.bind();
	m_reflectionFrameBuffer.detachTexture(GL_COLOR_ATTACHMENT0);

	m_reflectionTexture.freeGL();
	GlHelper::makeFloatColorTexture(m_reflectionTexture, m_size.x, m_size.y);
	m_reflectionTexture.initGL();
	m_reflectionFrameBuffer.attachTexture(&m_reflectionTexture, GL_COLOR_ATTACHMENT0);

	m_reflectionFrameBuffer.detachRenderBuffer(GL_STENCIL_ATTACHMENT);
	m_reflectionStencilBuffer.resize(m_size.x, m_size.y);
	m_reflectionFrameBuffer.attachRenderBuffer(&m_reflectionStencilBuffer, GL_STENCIL_ATTACHMENT);

	m_reflectionFrameBuffer.checkIntegrity();
	m_reflectionFrameBuffer.unbind();
}

//////////////////////////////////

ColorAndDepthRTL::ColorAndDepthRTL()
	: RenderTargetLayer("ColorAndDepth")
	, m_depthBuffer(400, 400, GL_DEPTH_COMPONENT24)
{
	// Setup texture
	GlHelper::makeFloatColorTexture(m_texture, m_size.x, m_size.y);
	m_texture.initGL();
	// Setup framebuffer
	m_frameBuffer.bind();
	m_frameBuffer.attachTexture(&m_texture, GL_COLOR_ATTACHMENT0);
	m_frameBuffer.attachRenderBuffer(&m_depthBuffer, GL_DEPTH_ATTACHMENT);
	m_frameBuffer.checkIntegrity();
	m_frameBuffer.unbind();
}

void ColorAndDepthRTL::bindFramebuffer(GLenum bindingStatus) const
{
	m_frameBuffer.bind(bindingStatus);
}

void ColorAndDepthRTL::unbindFramebuffer(GLenum bindingStatus) const
{
	m_frameBuffer.unbind(bindingStatus);
}

const GlHelper::Framebuffer & ColorAndDepthRTL::getFrameBuffer() const
{
	return m_frameBuffer;
}

GLuint ColorAndDepthRTL::getFinalFrame() const
{
	return m_texture.glId;
}

void ColorAndDepthRTL::onResize(const glm::vec2 & newSize)
{
	m_size = newSize;

	m_frameBuffer.bind();
	m_frameBuffer.detachTexture(GL_COLOR_ATTACHMENT0);

	m_texture.freeGL();
	GlHelper::makeFloatColorTexture(m_texture, m_size.x, m_size.y);
	m_texture.initGL();
	m_frameBuffer.attachTexture(&m_texture, GL_COLOR_ATTACHMENT0);

	m_frameBuffer.detachRenderBuffer(GL_DEPTH_ATTACHMENT);
	m_depthBuffer.resize(m_size.x, m_size.y);
	m_frameBuffer.attachRenderBuffer(&m_depthBuffer, GL_DEPTH_ATTACHMENT);

	m_frameBuffer.checkIntegrity();
	m_frameBuffer.unbind();
}

/////////////////////////////////////

RenderTarget::RenderTarget(std::shared_ptr<RenderTargetLayer> defaultLayer, float width, float height)
	: m_size(width, height)
{
	m_layers.push_back(defaultLayer);
	defaultLayer->onResize(m_size);
}

RenderTarget::RenderTarget(std::shared_ptr<RenderTargetLayer> firstLayer, std::shared_ptr<RenderTargetLayer> secondLayer, float width, float height)
	: m_size(width, height)
{
	m_layers.push_back(firstLayer);
	firstLayer->onResize(m_size);

	m_layers.push_back(secondLayer);
	secondLayer->onResize(m_size);
}

const glm::vec2 & RenderTarget::getSize() const
{
	return m_size;
}

void RenderTarget::setSize(const glm::vec2 & size)
{
	m_size = size;
	onResize();
}

bool RenderTarget::hasLayer(const std::string & layerName)
{
	return std::find_if(m_layers.begin(), m_layers.end(), [&layerName](const std::shared_ptr<RenderTargetLayer>& item) { return item->getName() == layerName; }) != m_layers.end();
}

bool RenderTarget::getLayerNameFromIndex(int idx, std::string & outLayerName) const
{
	if (idx >= 0 && idx < m_layers.size())
	{
		outLayerName = m_layers[idx]->getName();
		return true;
	}
	else
		return false;
}

void RenderTarget::bindFramebuffer(int layerIdx)
{
	if (layerIdx >= 0 && layerIdx < m_layers.size())
	{
		m_layers[layerIdx]->bindFramebuffer();
	}
}

void RenderTarget::unbindFramebuffer(int layerIdx)
{
	if (layerIdx >= 0 && layerIdx < m_layers.size())
	{
		m_layers[layerIdx]->unbindFramebuffer();
	}
}

bool RenderTarget::getFrameBuffer(int layerIdx, GlHelper::Framebuffer& outFrameBuffer) const
{
	if (layerIdx >= 0 && layerIdx < m_layers.size())
	{
		outFrameBuffer = m_layers[layerIdx]->getFrameBuffer();
		return true;
	}
	else
		return false;
}

GLuint RenderTarget::getFinalFrame(int layerIdx) const
{
	if (layerIdx >= 0 && layerIdx < m_layers.size())
	{
		return m_layers[layerIdx]->getFinalFrame();
	}
	else
		return -1;
}

void RenderTarget::addLayer(std::shared_ptr<RenderTargetLayer> newLayer)
{
	m_layers.push_back(newLayer);
}

bool RenderTarget::removeLayer(int layerIndex)
{
	if (layerIndex >= 0 && layerIndex < m_layers.size())
	{
		m_layers.erase(m_layers.begin() + layerIndex);
		return true;
	}
	else
		return false;
}

RenderTargetLayer * RenderTarget::getLayer(int layerIndex)
{
	if(layerIndex < 0 || layerIndex >= m_layers.size())
		return nullptr;
	else
		return m_layers[layerIndex].get();
}

RenderTargetLayer * RenderTarget::getLayer(const std::string & layerName)
{
	auto foundIt = std::find_if(m_layers.begin(), m_layers.end(), [&layerName](const std::shared_ptr<RenderTargetLayer>& item) { return item->getName() == layerName; });
	if (foundIt == m_layers.end())
		return nullptr;
	else
		return foundIt->get();
}

void RenderTarget::onResize()
{
	for (auto layer : m_layers)
	{
		layer->onResize(m_size);
	}
}
