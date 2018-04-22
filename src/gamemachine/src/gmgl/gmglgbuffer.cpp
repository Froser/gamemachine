#include "stdafx.h"
#include <GL/glew.h>
#include "gmglgbuffer.h"
#include "gmglgraphic_engine.h"
#include "foundation/gamemachine.h"
#include "shader_constants.h"
#include "gmgltexture.h"

#define GMEngine static_cast<GMGLGraphicEngine*>(GM.getGraphicEngine())

namespace
{
	constexpr GMint GEOMETRY_NUM = (GMint)GBufferGeometryType::EndOfGeometryType;
	constexpr GMint MATERIAL_NUM = (GMint)GBufferMaterialType::EndOfMaterialType;

	Array<const char*, GEOMETRY_NUM> g_GBufferGeometryUniformNames =
	{
		"deferred_light_pass_gPosition",
		"deferred_light_pass_gNormal",
		"deferred_light_pass_gNormal_eye",
		"deferred_light_pass_gTexAmbient",
		"deferred_light_pass_gTexDiffuse",
		"deferred_light_pass_gTangent_eye",
		"deferred_light_pass_gBitangent_eye",
		"deferred_light_pass_gNormalMap",
	};

	Array<const char*, MATERIAL_NUM> g_GBufferMaterialUniformNames =
	{
		"deferred_material_pass_gKa",
		"deferred_material_pass_gKd",
		"deferred_material_pass_gKs_gShininess", //gKs: 3, gShininess: 1
		"deferred_material_pass_gHasNormalMap_gRefractivity", //gHasNormalMap: 1, gRefractivity: 1
	};

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
			db->target = GL_TEXTURE_2D;

			GM_BEGIN_CHECK_GL_ERROR
			glGenTextures(1, &db->id);
			glBindTexture(GL_TEXTURE_2D, db->id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, d->desc.rect.width, d->desc.rect.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			db->texParamsSet = true;
			GM_END_CHECK_GL_ERROR
		}

		GLuint getTextureId()
		{
			D_BASE(d, Base);
			return d->id;
		}
	};
}

GMGLGBuffer::~GMGLGBuffer()
{
	dispose();
}

void GMGLGBuffer::adjustViewport()
{
	D(d);
	GMEngine->setViewport(d->viewport);
}

void GMGLGBuffer::beginPass()
{
	D(d);
	d->currentTurn = (GMint)GMGLDeferredRenderState::PassingGeometry;
	GMEngine->setRenderState((GMGLDeferredRenderState)d->currentTurn);
}

bool GMGLGBuffer::nextPass()
{
	D(d);
	GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(GM.getGraphicEngine());
	++d->currentTurn;
	engine->setRenderState((GMGLDeferredRenderState)d->currentTurn);
	if (d->currentTurn == GMGLGBuffer_TotalTurn)
		return false;
	return true;
}

void GMGLGBuffer::dispose()
{
	D(d);
	if (d->fbo[0])
	{
		glDeleteFramebuffers(GMGLGBuffer_TotalTurn, d->fbo);
		GM_ZeroMemory(d->fbo);
	}

	if (d->textures[0])
	{
		glDeleteTextures(GEOMETRY_NUM, d->textures);
		GM_ZeroMemory(d->textures);
	}

	if (d->materials[0])
	{
		glDeleteTextures(MATERIAL_NUM, d->materials);
		GM_ZeroMemory(d->materials);
	}

	if (d->depthBuffers[0])
	{
		glDeleteRenderbuffers(GMGLGBuffer_TotalTurn, d->depthBuffers);
		GM_ZeroMemory(d->depthBuffers);
	}
}

bool GMGLGBuffer::init(const GMRect& renderRect)
{
	D(d);
	GMRenderConfig renderConfig = GM.getConfigs().getConfig(GMConfigs::Render).asRenderConfig();

	d->renderRect = renderRect;
	d->renderWidth = renderRect.width;
	d->renderHeight = renderRect.height;
	d->viewport = { d->renderRect.x, d->renderRect.y, d->renderWidth, d->renderHeight };

	glGenFramebuffers(GMGLGBuffer_TotalTurn, d->fbo);
	if (!createFrameBuffers(GMGLDeferredRenderState::PassingGeometry, GEOMETRY_NUM, d->textures))
		return false;

	if (!createFrameBuffers(GMGLDeferredRenderState::PassingMaterial, MATERIAL_NUM, d->materials))
		return false;

	releaseBind();
	return true;
}

void GMGLGBuffer::bindForWriting()
{
	D(d);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d->fbo[d->currentTurn]);
}

void GMGLGBuffer::bindForReading()
{
	D(d);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, d->fbo[d->currentTurn]);
}

void GMGLGBuffer::releaseBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GMGLGBuffer::setReadBuffer(GBufferGeometryType textureType)
{
	D(d);
	GM_ASSERT(d->currentTurn == 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0 + (GMuint)textureType);
}

void GMGLGBuffer::setReadBuffer(GBufferMaterialType materialType)
{
	D(d);
	GM_ASSERT(d->currentTurn == 1);
	glReadBuffer(GL_COLOR_ATTACHMENT0 + (GMuint)materialType);
}

void GMGLGBuffer::newFrame()
{
	bindForWriting();
	GMGLGraphicEngine::newFrameOnCurrentFramebuffer();
	releaseBind();
}

void GMGLGBuffer::activateTextures()
{
	D(d);
	IShaderProgram* shaderProgram = GM.getGraphicEngine()->getShaderProgram();
	shaderProgram->useProgram();

	{
		for (GMuint i = 0; i < GEOMETRY_NUM; i++)
		{
			GM_BEGIN_CHECK_GL_ERROR
			shaderProgram->setInt(g_GBufferGeometryUniformNames[i], i);
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, d->textures[i]);
			GM_END_CHECK_GL_ERROR
		}
	}

	constexpr GMuint GEOMETRY_OFFSET = GEOMETRY_NUM;
	{
		for (GMuint i = 0; i < MATERIAL_NUM; i++)
		{
			GM_BEGIN_CHECK_GL_ERROR
			shaderProgram->setInt(g_GBufferMaterialUniformNames[i], GEOMETRY_OFFSET + i);
			glActiveTexture(GL_TEXTURE0 + GEOMETRY_OFFSET + i);
			glBindTexture(GL_TEXTURE_2D, d->materials[i]);
			GM_END_CHECK_GL_ERROR
		}
	}
}

void GMGLGBuffer::copyDepthBuffer(GLuint target)
{
	D(d);
	GM_BEGIN_CHECK_GL_ERROR
	glBindFramebuffer(GL_READ_FRAMEBUFFER, d->fbo[(GMint)GMGLDeferredRenderState::PassingGeometry]);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target);
	glBlitFramebuffer(0, 0, d->renderWidth, d->renderHeight, 0, 0, d->renderWidth, d->renderHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	GM_END_CHECK_GL_ERROR
}

bool GMGLGBuffer::createFrameBuffers(GMGLDeferredRenderState state, GMint textureCount, GLuint* textureArray)
{
	D(d);
	GMint s = (GMint)state;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d->fbo[s]);
	glGenTextures(textureCount, textureArray);
	for (GMint i = 0; i < textureCount; i++)
	{
		glBindTexture(GL_TEXTURE_2D, textureArray[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, d->renderWidth, d->renderHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textureArray[i], 0);
		GM_CHECK_GL_ERROR();
	}

	// If using STENCIL buffer:
	// NEVER EVER MAKE A STENCIL buffer. All GPUs and all drivers do not support an independent stencil buffer.
	// If you need a stencil buffer, then you need to make a Depth=24, Stencil=8 buffer, also called D24S8.
	// See https://www.khronos.org/opengl/wiki/Framebuffer_Object_Extension_Examples

	glGenRenderbuffers(1, &d->depthBuffers[s]);
	glBindRenderbuffer(GL_RENDERBUFFER, d->depthBuffers[s]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, d->renderWidth, d->renderHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, d->depthBuffers[s]);
	GM_CHECK_GL_ERROR();

	if (!drawBuffers(textureCount))
		return false;

	return true;
}

bool GMGLGBuffer::drawBuffers(GMuint count)
{
	Vector<GLuint> attachments;
	for (GMuint i = 0; i < count; i++)
	{
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	GM_BEGIN_CHECK_GL_ERROR
	glDrawBuffers(count, attachments.data());
	GM_END_CHECK_GL_ERROR

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		gm_error("FB incomplete error, status: 0x%x\n", status);
		return false;
	}
	return true;
}

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
		GM_ASSERT(suc);
		d->framebuffersCreated = suc;
	}
	
	if (d->framebuffersCreated)
	{
		GM_BEGIN_CHECK_GL_ERROR
		glBindFramebuffer(GL_FRAMEBUFFER, d->fbo);
		GM_END_CHECK_GL_ERROR
	}
}

void GMGLFramebuffers::unbind()
{
	GM_BEGIN_CHECK_GL_ERROR
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GM_END_CHECK_GL_ERROR
}

void GMGLFramebuffers::createDepthStencilBuffer(const GMFramebufferDesc& desc)
{
	D(d);
	// If using STENCIL buffer:
	// NEVER EVER MAKE A STENCIL buffer. All GPUs and all drivers do not support an independent stencil buffer.
	// If you need a stencil buffer, then you need to make a Depth=24, Stencil=8 buffer, also called D24S8.
	// See https://www.khronos.org/opengl/wiki/Framebuffer_Object_Extension_Examples

	GM_BEGIN_CHECK_GL_ERROR
	glBindFramebuffer(GL_FRAMEBUFFER, d->fbo);
	glGenRenderbuffers(1, &d->depthStencilBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, d->depthStencilBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, desc.rect.width, desc.rect.height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, d->depthStencilBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GM_END_CHECK_GL_ERROR
}

bool GMGLFramebuffers::createFramebuffers()
{
	D(d);
	glBindFramebuffer(GL_FRAMEBUFFER, d->fbo);
	Vector<GLuint> attachments;
	GMuint sz = d->framebuffers.size();
	for (GMuint i = 0; i < sz; i++)
	{
		GM_BEGIN_CHECK_GL_ERROR
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, d->framebuffers[i]->getTextureId(), 0);
		GM_END_CHECK_GL_ERROR
	}

	GM_BEGIN_CHECK_GL_ERROR
	glDrawBuffers(sz, attachments.data());
	GM_END_CHECK_GL_ERROR

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
	glBindFramebuffer(GL_FRAMEBUFFER, d->fbo);
	GM_BEGIN_CHECK_GL_ERROR
	glGetIntegerv(GL_STENCIL_WRITEMASK, &mask);
	glStencilMask(0xFF);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	GM_END_CHECK_GL_ERROR
	glStencilMask(mask);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

IFramebuffer* GMGLFramebuffers::getFramebuffer(GMuint index)
{
	D(d);
	GM_ASSERT(index < d->framebuffers.size());
	return d->framebuffers[index];
}