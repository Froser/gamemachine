#include "stdafx.h"
#include <GL/glew.h>
#include "gmglframebuffer.h"
#include "gmgltexture.h"
#include "gmengine/gmgraphicengine.h"
#include "foundation/gamemachine.h"
#include "gmengine/gmcsmhelper.h"
#include "gmglgraphic_engine.h"
#include "gmgltexture_p.h"

BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMGLFramebuffers)
{
	GMuint32 fbo = 0;
	GMuint32 depthStencilBuffer = 0;
	Vector<GMGLFramebuffer*> framebuffers;
	bool framebuffersCreated = false;
	GMGraphicEngine* engine = nullptr;
	GMFramebufferDesc desc;
	GMfloat clearColor[4];
	const IRenderContext* context = nullptr;
	void createFramebuffers();
};

GM_PRIVATE_OBJECT_UNALIGNED(GMGLFramebufferTexture)
{
	GMFramebufferDesc desc;
};

class GMGLFramebufferTexture : public GMGLTexture
{
	GM_DECLARE_PRIVATE(GMGLFramebufferTexture)
	GM_DECLARE_BASE(GMGLTexture)

public:
	GMGLFramebufferTexture(const GMFramebufferDesc& desc)
		: GMGLTexture(nullptr)
	{
		GM_CREATE_DATA();
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

GM_PRIVATE_OBJECT_UNALIGNED(GMGLFramebuffer)
{
	const IRenderContext* context = nullptr;
	GMTextureAsset texture;
};

GMGLFramebuffer::GMGLFramebuffer(const IRenderContext* context)
{
	GM_CREATE_DATA();
	D(d);
	d->context = context;
}

GMGLFramebuffer::~GMGLFramebuffer()
{

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
	GM_CREATE_DATA();

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

	// 需要手动清理error记录，否则不能直接拿glGetError来判断是否是否成功，它会拿最后一个错误。
	GMGLGraphicEngine::clearGLErrors();

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
	d->createFramebuffers();
	glBindFramebuffer(GL_FRAMEBUFFER, d->fbo);
	setViewport();
}

void GMGLFramebuffers::bind()
{
	D(d);
	d->createFramebuffers();
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


const GMFramebufferDesc& GMGLFramebuffers::getDesc()
{
	D(d);
	return d->desc;
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

void GMGLFramebuffersPrivate::createFramebuffers()
{
	if (!framebuffersCreated)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		Vector<GLuint> attachments;
		GMsize_t sz = framebuffers.size();
		for (GMsize_t i = 0; i < sz; i++)
		{
			GMuint32 _i = gm_sizet_to_uint(i);
			attachments.push_back(GL_COLOR_ATTACHMENT0 + _i);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + _i, GL_TEXTURE_2D, framebuffers[_i]->getTextureId(), 0);
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
		framebuffersCreated = true;
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

GM_PRIVATE_OBJECT_UNALIGNED(GMGLShadowMapTexture)
{
	GMuint32 textureId;
};

GMGLShadowMapTexture::GMGLShadowMapTexture(GMuint32 textureId)
{
	GM_CREATE_DATA();

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


GM_PRIVATE_OBJECT_ALIGNED(GMGLShadowFramebuffers)
{
	GMint32 width = 0;
	GMint32 height = 0;
	GMTextureAsset shadowMapTexture;
	GMShadowSourceDesc shadowSource;
	GMfloat cascadeEndClip[GMGraphicEngine::getMaxCascades()] = { 0 };
	GMCascadeLevel currentViewport;

	struct
	{
		GMfloat topLeftX;
		GMfloat topLeftY;
		GMfloat width;
		GMfloat height;
	} viewports[GMGraphicEngine::getMaxCascades()] = { 0 };
};

GMGLShadowFramebuffers::GMGLShadowFramebuffers(const IRenderContext* context)
	: GMGLFramebuffers(context)
{
	GM_CREATE_DATA();
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

	for (GMCascadeLevel i = 0; i < d->shadowSource.cascades; ++i)
	{
		// 创建每一个cascade的viewport
		d->viewports[i].height = d->shadowSource.height;
		d->viewports[i].width = d->shadowSource.width;
		d->viewports[i].topLeftX = d->shadowSource.width * i;
		d->viewports[i].topLeftY = 0;

		setEachCascadeEndClip(i);
	}

	return true;
}

void GMGLShadowFramebuffers::use()
{
	D(d);
	D_BASE(db, Base);
	glBindFramebuffer(GL_FRAMEBUFFER, db->fbo);

	// 选择当前的viewport
	const auto& viewport = d->viewports[d->currentViewport];
	glViewport(viewport.topLeftX, viewport.topLeftY, viewport.width, viewport.height);
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

void GMGLShadowFramebuffers::setShadowSource(const GMShadowSourceDesc& shadowSource)
{
	D(d);
	d->shadowSource = shadowSource;
}

GMTextureAsset GMGLShadowFramebuffers::getShadowMapTexture()
{
	D(d);
	return d->shadowMapTexture;
}

GMint32 GMGLShadowFramebuffers::getShadowMapHeight()
{
	D(d);
	return d->height;
}

GMint32 GMGLShadowFramebuffers::getShadowMapWidth()
{
	D(d);
	return d->width;
}

GMCascadeLevel GMGLShadowFramebuffers::cascadedBegin()
{
	D(d);
	return 0;
}

GMCascadeLevel GMGLShadowFramebuffers::cascadedEnd()
{
	D(d);
	return d->shadowSource.cascades;
}

void GMGLShadowFramebuffers::applyCascadedLevel(GMCascadeLevel vp)
{
	D(d);
	d->currentViewport = vp;
	use();
}

GMCascadeLevel GMGLShadowFramebuffers::currentLevel()
{
	D(d);
	return d->currentViewport;
}

GMfloat GMGLShadowFramebuffers::getEndClip(GMCascadeLevel level)
{
	D(d);
	return d->cascadeEndClip[level];
}

void GMGLShadowFramebuffers::setEachCascadeEndClip(GMCascadeLevel level)
{
	D(d);
	D_BASE(db, Base);
	// 根据相机透视矩阵，来算出每个裁剪范围
	const GMCamera& camera = d->shadowSource.camera;
	GMfloat intervalBegin = 0, intervalEnd = 0;
	GMCSMHelper::getFrustumIntervals(db->context->getEngine()->getCamera(), d->shadowSource, level, intervalBegin, intervalEnd);
	GMVec4 view = { 0, 0, intervalEnd, 1.f };
	GMVec4 clip = view * camera.getProjectionMatrix();
	d->cascadeEndClip[level] = clip.getZ();
}

END_NS