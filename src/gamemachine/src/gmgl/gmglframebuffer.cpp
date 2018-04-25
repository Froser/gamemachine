#include "stdafx.h"
#include <gl/glew.h>
#include "gmglframebuffer.h"
#include "gmgltexture.h"
#include "gmengine/gmgraphicengine.h"
#include "foundation/gamemachine.h"

GM_PRIVATE_OBJECT(GMGLFramebufferTexture)
{
	GMFramebufferDesc desc;
};

class GMGLFramebufferTexture : public GMGLTexture
{
	DECLARE_PRIVATE_AND_BASE(GMGLFramebufferTexture, GMGLTexture);

public:
	GMGLFramebufferTexture(const GMFramebufferDesc& desc)
		: GMGLTexture(nullptr)
	{
		D(d);
		d->desc = desc;
	}

	virtual void init() override
	{
		D(d);
		D_BASE(db, Base);
		GLenum format;
		if (d->desc.framebufferFormat == GMFramebufferFormat::R8G8B8A8_UNORM)
		{
			format = GL_RGBA8;
		}
		else if (d->desc.framebufferFormat == GMFramebufferFormat::R32G32B32A32_FLOAT)
		{
			format = GL_RGBA32F;
		}
		else
		{
			format = GL_RGBA8;
			GM_ASSERT(!"Unsupported format.");
			gm_error("Unsupported format.");
		}

		db->target = GL_TEXTURE_2D;

		glGenTextures(1, &db->id);
		glBindTexture(GL_TEXTURE_2D, db->id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, d->desc.rect.width, d->desc.rect.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		db->texParamsSet = true;
	}

	GLuint getTextureId()
	{
		D_BASE(d, Base);
		return d->id;
	}
};

GMGLFramebuffer::~GMGLFramebuffer()
{
	D(d);
	GM_delete(d->texture);
}

bool GMGLFramebuffer::init(const GMFramebufferDesc& desc)
{
	D(d);
	d->texture = new GMGLFramebufferTexture(desc);
	d->texture->init();
	return true;
}

ITexture* GMGLFramebuffer::getTexture()
{
	D(d);
	return d->texture;
}

GMuint GMGLFramebuffer::getTextureId()
{
	D(d);
	GM_ASSERT(dynamic_cast<GMGLFramebufferTexture*>(d->texture));
	GMGLFramebufferTexture* texture = static_cast<GMGLFramebufferTexture*>(d->texture);
	return texture->getTextureId();
}

GMGLFramebuffers::GMGLFramebuffers()
{
	D(d);
	d->engine = gm_cast<GMGraphicEngine*>(GM.getGraphicEngine());
}

GMGLFramebuffers::~GMGLFramebuffers()
{
	D(d);
	for (auto framebuffer : d->framebuffers)
	{
		GM_delete(framebuffer);
	}

	glDeleteFramebuffers(1, &d->fbo);
	d->fbo = 0;

	glDeleteRenderbuffers(1, &d->depthStencilBuffer);
	d->depthStencilBuffer = 0;
}

bool GMGLFramebuffers::init(const GMFramebufferDesc& desc)
{
	D(d);
	glGenFramebuffers(1, &d->fbo);
	createDepthStencilBuffer(desc);
	return glGetError() == GL_NO_ERROR;
}

void GMGLFramebuffers::addFramebuffer(AUTORELEASE IFramebuffer* framebuffer)
{
	D(d);
	GM_ASSERT(dynamic_cast<GMGLFramebuffer*>(framebuffer));
	GMGLFramebuffer* glFramebuffer = static_cast<GMGLFramebuffer*>(framebuffer);
	d->framebuffers.push_back(glFramebuffer);
}

void GMGLFramebuffers::bind()
{
	D(d);
	if (!d->framebuffersCreated)
	{
		bool suc = createFramebuffers();
		d->framebuffersCreated = suc;
	}

	if (d->framebuffersCreated)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, d->fbo);
		d->engine->getFramebuffersStack().push(this);
	}
}

void GMGLFramebuffers::unbind()
{
	D(d);
	GMFramebuffersStack& stack = d->engine->getFramebuffersStack();
	IFramebuffers* currentFramebuffers = stack.pop();
	if (currentFramebuffers != this)
	{
		GM_ASSERT(false);
		gm_error("Cannot unbind framebuffer because current framebuffer isn't this framebuffer.");
	}
	else
	{
		IFramebuffers* lastFramebuffers = stack.peek();
		if (lastFramebuffers)
			lastFramebuffers->bind();
		else
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void GMGLFramebuffers::createDepthStencilBuffer(const GMFramebufferDesc& desc)
{
	D(d);
	// If using STENCIL buffer:
	// NEVER EVER MAKE A STENCIL buffer. All GPUs and all drivers do not support an independent stencil buffer.
	// If you need a stencil buffer, then you need to make a Depth=24, Stencil=8 buffer, also called D24S8.
	// See https://www.khronos.org/opengl/wiki/Framebuffer_Object_Extension_Examples

	glBindFramebuffer(GL_FRAMEBUFFER, d->fbo);
	glGenRenderbuffers(1, &d->depthStencilBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, d->depthStencilBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, desc.rect.width, desc.rect.height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, d->depthStencilBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool GMGLFramebuffers::createFramebuffers()
{
	D(d);
	glBindFramebuffer(GL_FRAMEBUFFER, d->fbo);
	Vector<GLuint> attachments;
	GMuint sz = d->framebuffers.size();
	for (GMuint i = 0; i < sz; i++)
	{
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, d->framebuffers[i]->getTextureId(), 0);
	}

	glDrawBuffers(sz, attachments.data());

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		gm_error("FB incomplete error, status: 0x%x\n", status);
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

void GMGLFramebuffers::clear()
{
	D(d);
	GLint mask;
	bind();
	glGetIntegerv(GL_STENCIL_WRITEMASK, &mask);
	glStencilMask(0xFF);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(mask);
	unbind();
}

IFramebuffer* GMGLFramebuffers::getFramebuffer(GMuint index)
{
	D(d);
	GM_ASSERT(index < d->framebuffers.size());
	return d->framebuffers[index];
}

GMuint GMGLFramebuffers::count()
{
	D(d);
	return d->framebuffers.size();
}