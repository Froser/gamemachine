#include "stdafx.h"
#include <gl/glew.h>
#include "gmglframebuffer.h"
#include "gmgltexture.h"
#include "gmengine/gmgraphicengine.h"
#include "foundation/gamemachine.h"

#ifdef max
#undef max
#endif

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
		glTexImage2D(GL_TEXTURE_2D, 0, format, d->desc.rect.width, d->desc.rect.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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

class GMGLDefaultFramebuffers : public GMGLFramebuffers
{
public:
	GMGLDefaultFramebuffers()
	{
		D_BASE(d, GMGLFramebuffers);
		d->fbo = 0;
		d->framebuffersCreated = true;
	}

public:
	virtual bool init(const GMFramebuffersDesc& desc) override
	{
		return false;
	}

	virtual void addFramebuffer(AUTORELEASE IFramebuffer* framebuffer) override
	{
	}

	virtual GMsize_t count() override
	{
		return 1;
	}

	virtual IFramebuffer* getFramebuffer(GMsize_t) override
	{
		return nullptr;
	}

	virtual GMuint framebufferId() override
	{
		return 0;
	}

private:
	virtual void setViewport() override
	{
		D(d);
		GMRect rect = GM.getGameMachineRunningStates().renderRect;
		glViewport(0, 0, rect.width, rect.height);
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

bool GMGLFramebuffers::init(const GMFramebuffersDesc& desc)
{
	D(d);
	d->desc.rect = desc.rect;
	d->clearColor[0] = desc.clearColor[0];
	d->clearColor[1] = desc.clearColor[1];
	d->clearColor[2] = desc.clearColor[2];
	d->clearColor[3] = desc.clearColor[3];

	glGenFramebuffers(1, &d->fbo);
	createDepthStencilBuffer(d->desc);
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
		setViewport();
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
			getDefaultFramebuffers()->bind();
	}
}

void GMGLFramebuffers::copyDepthStencilFramebuffer(IFramebuffers* dest)
{
	D(d);
	GLint cache;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &cache);

	GMGLFramebuffers* destFramebuffers = gm_cast<GMGLFramebuffers*>(dest);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferId());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFramebuffers->framebufferId());
	glBlitFramebuffer(
		0, 0, d->desc.rect.width, d->desc.rect.height,
		0, 0, d->desc.rect.width, d->desc.rect.height,
		GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, cache);
}

GMuint GMGLFramebuffers::framebufferId()
{
	D(d);
	return d->fbo;
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
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, desc.rect.width, desc.rect.height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, d->depthStencilBuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, d->depthStencilBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GMGLFramebuffers::setViewport()
{
	D(d);
	glViewport(0, 0, d->desc.rect.width, d->desc.rect.height);
}

bool GMGLFramebuffers::createFramebuffers()
{
	D(d);
	glBindFramebuffer(GL_FRAMEBUFFER, d->fbo);
	Vector<GLuint> attachments;
	GMsize_t sz = d->framebuffers.size();
	GM_ASSERT(sz < std::numeric_limits<GMuint>::max());
	for (GMsize_t i = 0; i < sz; i++)
	{
		GMuint _i = (GMuint)i;
		attachments.push_back(GL_COLOR_ATTACHMENT0 + _i);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + _i, GL_TEXTURE_2D, d->framebuffers[_i]->getTextureId(), 0);
	}
	glDrawBuffers((GLsizei)sz, attachments.data());

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		gm_error("FB incomplete error, status: 0x%x\n", status);
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

void GMGLFramebuffers::clear(GMFramebuffersClearType type)
{
	D(d);
	GMuint iType = (GMuint)type;
	GLenum mask = 0x00;
	if (iType & (GMuint)GMFramebuffersClearType::Color)
		mask |= GL_COLOR_BUFFER_BIT;
	if (iType & (GMuint)GMFramebuffersClearType::Depth)
		mask |= GL_DEPTH_BUFFER_BIT;
	if (iType & (GMuint)GMFramebuffersClearType::Stencil)
		mask |= GL_STENCIL_BUFFER_BIT;

	if (mask)
	{
		bind();
		glClearColor(d->clearColor[0], d->clearColor[1], d->clearColor[2], d->clearColor[3]);
		if (mask & GL_STENCIL_BUFFER_BIT)
		{
			GLint stencilMask;
			glGetIntegerv(GL_STENCIL_WRITEMASK, &stencilMask);
			glStencilMask(0xFF);
			glClear(mask);
			glStencilMask(stencilMask);
		}
		else
		{
			glClear(mask);
		}
		unbind();
	}
}

IFramebuffer* GMGLFramebuffers::getFramebuffer(GMsize_t index)
{
	D(d);
	GM_ASSERT(index < d->framebuffers.size());
	return d->framebuffers[index];
}

GMsize_t GMGLFramebuffers::count()
{
	D(d);
	return d->framebuffers.size();
}

IFramebuffers* GMGLFramebuffers::getDefaultFramebuffers()
{
	static GMGLDefaultFramebuffers s_defaultFramebuffers;
	return &s_defaultFramebuffers;
}

GMGLShadowMapTexture::GMGLShadowMapTexture(GMuint textureId)
{
	D(d);
	d->textureId = textureId;
}

GMGLShadowMapTexture::~GMGLShadowMapTexture()
{
	D(d);
	glDeleteTextures(1, &d->textureId);
}

void GMGLShadowMapTexture::useTexture(GMint)
{
	D(d);
	glActiveTexture(GL_TEXTURE0 + GMTextureRegisterQuery<GMTextureType::ShadowMap>::Value);
	glBindTexture(GL_TEXTURE_2D, d->textureId);
}

void GMGLShadowMapTexture::init()
{
}

GMGLShadowFramebuffers::~GMGLShadowFramebuffers()
{
	D(d);
	GM_delete(d->shadowMapTexture);
}

bool GMGLShadowFramebuffers::init(const GMFramebuffersDesc& desc)
{
	bool b = Base::init(desc);
	if (!b)
		return false;

	D(d);
	D_BASE(db, Base);
	d->width = desc.rect.width;
	d->height = desc.rect.height;
	return true;
}

void GMGLShadowFramebuffers::bind()
{
	D_BASE(d, Base);
	glBindFramebuffer(GL_FRAMEBUFFER, d->fbo);
	setViewport();
	d->engine->getFramebuffersStack().push(this);
}

void GMGLShadowFramebuffers::createDepthStencilBuffer(const GMFramebufferDesc& desc)
{
	D(d);
	D_BASE(db, Base);
	GLuint shadowMapTextureId = 0;
	glGenTextures(1, &shadowMapTextureId);
	glBindTexture(GL_TEXTURE_2D, shadowMapTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, desc.rect.width, desc.rect.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, db->fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTextureId, 0);
	glReadBuffer(GL_NONE);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GM_ASSERT(shadowMapTextureId != 0);
	d->shadowMapTexture = new GMGLShadowMapTexture(shadowMapTextureId);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	GM_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);
}