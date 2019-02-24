#include "stdafx.h"
#include <GL/glew.h>
#include "gmglframebuffer.h"
#include "gmgltexture.h"
#include "gmengine/gmgraphicengine.h"
#include "foundation/gamemachine.h"

GM_PRIVATE_OBJECT_UNALIGNED(GMGLFramebufferTexture)
{
	GMFramebufferDesc desc;
};

class GMGLFramebufferTexture : public GMGLTexture
{
	GM_DECLARE_PRIVATE_NGO(GMGLFramebufferTexture)
	typedef GMGLTexture Base;

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
			gm_error(gm_dbg_wrap("Unsupported format."));
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

BEGIN_NS
class GMGLDefaultFramebuffers : public GMGLFramebuffers
{
public:
	GMGLDefaultFramebuffers(const IRenderContext* context)
		: GMGLFramebuffers(context)
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

	virtual GMuint32 framebufferId() override
	{
		return 0;
	}

private:
	virtual void setViewport() override
	{
		D(d);
		GMRect rect = getContext()->getWindow()->getWindowStates().renderRect;
		glViewport(0, 0, rect.width, rect.height);
	}
};
END_NS

GMGLFramebuffer::GMGLFramebuffer(const IRenderContext* context)
{
	D(d);
	d->context = context;
}

bool GMGLFramebuffer::init(const GMFramebufferDesc& desc)
{
	D(d);
	GMGLFramebufferTexture* texture = new GMGLFramebufferTexture(desc);
	texture->init();
	d->texture = GMAsset(GMAssetType::Texture, texture);
	return true;
}

void GMGLFramebuffer::getTexture(REF GMTextureAsset& texture)
{
	D(d);
	texture = d->texture;
}

const IRenderContext* GMGLFramebuffer::getContext()
{
	D(d);
	return d->context;
}

GMuint32 GMGLFramebuffer::getTextureId()
{
	D(d);
	GM_ASSERT(dynamic_cast<GMGLFramebufferTexture*>(d->texture.getTexture()));
	GMGLFramebufferTexture* texture = d->texture.get<GMGLFramebufferTexture*>();
	return texture->getTextureId();
}

GMGLFramebuffers::GMGLFramebuffers(const IRenderContext* context)
{
	D(d);
	d->context = context;
	d->engine = gm_cast<GMGraphicEngine*>(context->getEngine());
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

void GMGLFramebuffers::use()
{
	D(d);
	createFramebuffers();
	glBindFramebuffer(GL_FRAMEBUFFER, d->fbo);
	setViewport();
}

void GMGLFramebuffers::bind()
{
	D(d);
	createFramebuffers();
	if (d->framebuffersCreated)
	{
		use();
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
		gm_error(gm_dbg_wrap("Cannot unbind framebuffer because current framebuffer isn't this framebuffer."));
	}
	else
	{
		IFramebuffers* lastFramebuffers = stack.peek();
		if (lastFramebuffers)
			lastFramebuffers->use();
		else
			d->context->getEngine()->getDefaultFramebuffers()->use();
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

void GMGLFramebuffers::setClearColor(const GMfloat rgba[4])
{
	D(d);
	memcpy_s(d->clearColor, sizeof(d->clearColor), rgba, sizeof(d->clearColor));
}

const IRenderContext* GMGLFramebuffers::getContext()
{
	D(d);
	return d->context;
}

GMuint32 GMGLFramebuffers::framebufferId()
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

void GMGLFramebuffers::createFramebuffers()
{
	D(d);
	if (!d->framebuffersCreated)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, d->fbo);
		Vector<GLuint> attachments;
		GMsize_t sz = d->framebuffers.size();
		for (GMsize_t i = 0; i < sz; i++)
		{
			GMuint32 _i = gm_sizet_to_uint(i);
			attachments.push_back(GL_COLOR_ATTACHMENT0 + _i);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + _i, GL_TEXTURE_2D, d->framebuffers[_i]->getTextureId(), 0);
		}
		glDrawBuffers((GLsizei)sz, attachments.data());

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			gm_error(gm_dbg_wrap("GMGLFramebuffers::createFramebuffers: FB incomplete error, status: {0}"), GMString(static_cast<GMint32>(status)));
			GM_ASSERT(false);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		d->framebuffersCreated = true;
	}
}

void GMGLFramebuffers::clear(GMFramebuffersClearType type)
{
	D(d);
	GMuint32 iType = (GMuint32)type;
	GLenum mask = 0x00;
	if (iType & (GMuint32)GMFramebuffersClearType::Color)
		mask |= GL_COLOR_BUFFER_BIT;
	if (iType & (GMuint32)GMFramebuffersClearType::Depth)
		mask |= GL_DEPTH_BUFFER_BIT;
	if (iType & (GMuint32)GMFramebuffersClearType::Stencil)
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

IFramebuffers* GMGLFramebuffers::createDefaultFramebuffers(const IRenderContext* context)
{
	return new GMGLDefaultFramebuffers(context);
}

GMGLShadowMapTexture::GMGLShadowMapTexture(GMuint32 textureId)
{
	D(d);
	d->textureId = textureId;
}

GMGLShadowMapTexture::~GMGLShadowMapTexture()
{
	D(d);
	glDeleteTextures(1, &d->textureId);
}

void GMGLShadowMapTexture::useTexture(GMint32)
{
	D(d);
	glActiveTexture(GL_TEXTURE0 + GMTextureRegisterQuery<GMTextureType::ShadowMap>::Value);
	glBindTexture(GL_TEXTURE_2D, d->textureId);
}

void GMGLShadowMapTexture::init()
{
}

GMGLShadowFramebuffers::GMGLShadowFramebuffers(const IRenderContext* context)
	: GMGLFramebuffers(context)
{
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

void GMGLShadowFramebuffers::use()
{
	D_BASE(d, Base);
	glBindFramebuffer(GL_FRAMEBUFFER, d->fbo);
	setViewport();
}


void gm::GMGLShadowFramebuffers::use(GMsize_t index)
{
	//TODO
}

void GMGLShadowFramebuffers::bind()
{
	D_BASE(d, Base);
	use();
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
	d->shadowMapTexture = GMAsset(GMAssetType::Texture, new GMGLShadowMapTexture(shadowMapTextureId));

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	GM_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);
}
