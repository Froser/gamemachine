#include "stdafx.h"
#include "gmglgbuffer.h"
#include "gmglgraphic_engine.h"
#include "foundation/gamemachine.h"

constexpr GMint GEOMETRY_NUM = (GMint)GBufferGeometryType::EndOfGeometryType;
constexpr GMint MATERIAL_NUM = (GMint)GBufferMaterialType::EndOfMaterialType;

Array<const char*, GEOMETRY_NUM> g_GBufferGeometryUniformNames =
{
	"gPosition",
	"gNormal_eye",
	"gTexAmbient",
	"gTexDiffuse",
	"gTangent_eye",
	"gBitangent_eye",
	"gNormalMap",
};

Array<const char*, MATERIAL_NUM> g_GBufferMaterialUniformNames =
{
	"gKa",
	"gKd",
	"gKs",
	"gShininess",
	"gHasNormalMap",
};

GMGLGBuffer::GMGLGBuffer()
{
}

GMGLGBuffer::~GMGLGBuffer()
{
	dispose();
}

void GMGLGBuffer::beginPass()
{
	D(d);
	d->currentTurn = (GMint)GMGLDeferredRenderState::PassingGeometry;
	GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(GM.getGraphicEngine());
	engine->setRenderState((GMGLDeferredRenderState)d->currentTurn);
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

	if (d->fbo)
	{
		glDeleteFramebuffers(GMGLGBuffer_TotalTurn, d->fbo);
		GM_ZeroMemory(d->fbo);
	}

	if (d->textures[0] != 0)
	{
		glDeleteTextures(GEOMETRY_NUM, d->textures);
		GM_ZeroMemory(d->textures);
	}

	if (d->materials[0] != 0)
	{
		glDeleteTextures(MATERIAL_NUM, d->materials);
		GM_ZeroMemory(d->materials);
	}

	if (d->depthBuffers != 0)
	{
		glDeleteRenderbuffers(GMGLGBuffer_TotalTurn, d->depthBuffers);
		GM_ZeroMemory(d->depthBuffers);
	}
}

bool GMGLGBuffer::init(GMuint windowWidth, GMuint windowHeight)
{
	D(d);
	d->windowWidth = windowWidth;
	d->windowHeight = windowHeight;

	bool success = false;
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
	ASSERT(d->currentTurn == 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0 + (GMuint)textureType);
}

void GMGLGBuffer::setReadBuffer(GBufferMaterialType materialType)
{
	D(d);
	ASSERT(d->currentTurn == 1);
	glReadBuffer(GL_COLOR_ATTACHMENT0 + (GMuint)materialType);
}

void GMGLGBuffer::newFrame()
{
	bindForWriting();
	GMGLGraphicEngine::newFrameOnCurrentContext();
	releaseBind();
}

void GMGLGBuffer::activateTextures(GMGLShaderProgram* shaderProgram)
{
	D(d);

	{
		for (GMuint i = 0; i < GEOMETRY_NUM; i++)
		{
			shaderProgram->setInt(g_GBufferGeometryUniformNames[i], i);
			GM_CHECK_GL_ERROR();
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, d->textures[i]);
			GM_CHECK_GL_ERROR();
		}
	}

	constexpr GMuint GEOMETRY_OFFSET = GEOMETRY_NUM;
	{
		for (GMuint i = 0; i < MATERIAL_NUM; i++)
		{
			shaderProgram->setInt(g_GBufferMaterialUniformNames[i], GEOMETRY_OFFSET + i);
			GM_CHECK_GL_ERROR();
			glActiveTexture(GL_TEXTURE0 + GEOMETRY_OFFSET + i);
			glBindTexture(GL_TEXTURE_2D, d->materials[i]);
			GM_CHECK_GL_ERROR();
		}
	}
}

void GMGLGBuffer::copyDepthBuffer()
{
	D(d);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, d->fbo[(GMint)GMGLDeferredRenderState::PassingGeometry]);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, d->windowWidth, d->windowHeight, 0, 0, d->windowWidth, d->windowHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GM_CHECK_GL_ERROR();
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, d->windowWidth, d->windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textureArray[i], 0);
		GM_CHECK_GL_ERROR();
	}

	// If using STENCIL buffer:
	// EVER EVER MAKE A STENCIL buffer. All GPUs and all drivers do not support an independent stencil buffer.
	// If you need a stencil buffer, then you need to make a Depth=24, Stencil=8 buffer, also called D24S8.
	// See https://www.khronos.org/opengl/wiki/Framebuffer_Object_Extension_Examples

	glGenRenderbuffers(1, &d->depthBuffers[s]);
	glBindRenderbuffer(GL_RENDERBUFFER, d->depthBuffers[s]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, d->windowWidth, d->windowHeight);
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

	glDrawBuffers(count, attachments.data());
	GM_CHECK_GL_ERROR();

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		gm_error("FB incomplete error, status: 0x%x\n", status);
		return false;
	}
	return true;
}