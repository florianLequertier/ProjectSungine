#pragma once

#include <map>
#include "Texture.h"
#include "ErrorHandler.h"

namespace GlHelper {

class Renderbuffer
{

private:
	GLuint m_glId;
	GLenum m_internalFormat;

public:

	Renderbuffer()
		: m_internalFormat(0)
	{
		glGenRenderbuffers(1, &m_glId);
	}

	Renderbuffer(float width, float height, GLenum internalFormat)
		: m_internalFormat(internalFormat)
	{
		glGenRenderbuffers(1, &m_glId);
		glBindRenderbuffer(GL_RENDERBUFFER, m_glId);
		glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void init(float width, float height, GLenum internalFormat)
	{
		m_internalFormat = internalFormat;

		glBindRenderbuffer(GL_RENDERBUFFER, m_glId);
		glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	~Renderbuffer()
	{
		glDeleteRenderbuffers(1, &m_glId);
	}

	void resize(float width, float height)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, m_glId);
		glRenderbufferStorage(GL_RENDERBUFFER, m_internalFormat, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	GLuint getId() const
	{
		return m_glId;
	}

	void bind()
	{
		glBindRenderbuffer(GL_RENDERBUFFER, m_glId);
	}
};


class Framebuffer 
{
public:
	enum AttachmentTypes {
		COLOR = GL_COLOR_ATTACHMENT0,
		DEPTH_STENCIL = GL_DEPTH_STENCIL_ATTACHMENT,
		DEPTH = GL_DEPTH_ATTACHMENT,
		STENCIL = GL_STENCIL_ATTACHMENT
	};

private:
	GLuint m_glId;
	std::map<GLenum, const Texture*> m_attachedTextures;
	std::map<GLenum, const Renderbuffer*> m_attachedRenderbuffers;

public:

	Framebuffer()
	{
		glGenFramebuffers(1, &m_glId);
	}

	~Framebuffer()
	{
		glDeleteFramebuffers(1, &m_glId);
	}

	void setDrawBuffers(int count, GLenum* datas)
	{
		glDrawBuffers(count, datas);
	}

	void setDrawBuffer(GLenum data)
	{
		glDrawBuffer(data);
	}

	void checkIntegrity()
	{
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			PRINT_ERROR("Error when building framebuffer\n opengl error enum : " + status);
		}
	}

	GLuint getId() const
	{
		return m_glId;
	}

	void bind(GLenum bindingStatus = GL_FRAMEBUFFER) const
	{
		glBindFramebuffer(bindingStatus, m_glId);
	}

	void unbind(GLenum bindingStatus = GL_FRAMEBUFFER) const
	{
		glBindFramebuffer(bindingStatus, 0);
	}

	void attachTexture(const Texture* textureToAttach, AttachmentTypes attachment, int mipMapLevel = 0, int colorAttachmentOffset = 0)
	{
		GLenum attachementType = (attachment == AttachmentTypes::COLOR) ? (GLenum)(attachment + colorAttachmentOffset) : (GLenum)attachment;
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachementType, GL_TEXTURE_2D, textureToAttach->glId, mipMapLevel);

		m_attachedTextures[attachementType] = textureToAttach;
	}

	void attachRenderBuffer(const Renderbuffer* renderBufferToAttach, AttachmentTypes attachmentType)
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, renderBufferToAttach->getId());

		m_attachedRenderbuffers[attachmentType] = renderBufferToAttach;
	}

	void attachTexture(const Texture* textureToAttach, GLenum attachmentType, int mipMapLevel = 0, int colorAttachmentOffset = 0)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, textureToAttach->glId, mipMapLevel);

		m_attachedTextures[attachmentType] = textureToAttach;
	}

	void detachTexture(GLenum attachmentType, int mipMapLevel = 0, int colorAttachmentOffset = 0)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, 0, mipMapLevel);

		m_attachedTextures[attachmentType] = nullptr;
	}

	void attachRenderBuffer(const Renderbuffer* renderBufferToAttach, GLenum attachmentType)
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, renderBufferToAttach->getId());

		m_attachedRenderbuffers[attachmentType] = renderBufferToAttach;
	}

	void detachRenderBuffer(GLenum attachmentType)
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, 0);

		m_attachedRenderbuffers[attachmentType] = nullptr;
	}

	const Texture* getAttachedTexture(GLenum attachmentType) const
	{
		auto& found = m_attachedTextures.find(attachmentType);
		if (found != m_attachedTextures.end())
			return found->second;
		else
			return nullptr;
	}

};

}