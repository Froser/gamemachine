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
		"deferred_material_pass_gKs_gShininess", //gKs: 3, gShininess: 1
		"deferred_material_pass_gHasNormalMap_gRefractivity", //gHasNormalMap: 1, gRefractivity: 1
	};
}

GMGLGBufferDep::~GMGLGBufferDep()
{
	dispose();
}

void GMGLGBufferDep::adjustViewport()
{
	D(d);
	GMEngine->setViewport(d->viewport);
}

void GMGLGBufferDep::beginPass()
{
	D(d);
	d->currentTurn = (GMint)GMGLDeferredRenderState::PassingGeometry;
	GMEngine->setRenderState((GMGLDeferredRenderState)d->currentTurn);
}

bool GMGLGBufferDep::nextPass()
{
	D(d);
	GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(GM.getGraphicEngine());
	++d->currentTurn;
	engine->setRenderState((GMGLDeferredRenderState)d->currentTurn);
	if (d->currentTurn == GMGLGBuffer_TotalTurn)
		return false;
	return true;
}

void GMGLGBufferDep::dispose()
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

bool GMGLGBufferDep::init(const GMRect& renderRect)
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

void GMGLGBufferDep::bindForWriting()
{
	D(d);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d->fbo[d->currentTurn]);
}

void GMGLGBufferDep::bindForReading()
{
	D(d);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, d->fbo[d->currentTurn]);
}

void GMGLGBufferDep::releaseBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GMGLGBufferDep::setReadBuffer(GBufferGeometryType textureType)
{
	D(d);
	GM_ASSERT(d->currentTurn == 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0 + (GMuint)textureType);
}

void GMGLGBufferDep::setReadBuffer(GBufferMaterialType materialType)
{
	D(d);
	GM_ASSERT(d->currentTurn == 1);
	glReadBuffer(GL_COLOR_ATTACHMENT0 + (GMuint)materialType);
}

void GMGLGBufferDep::newFrame()
{
	bindForWriting();
	GMGLGraphicEngine::newFrameOnCurrentFramebuffer();
	releaseBind();
}

void GMGLGBufferDep::activateTextures()
{
	D(d);
	GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(GM.getGraphicEngine());
	IShaderProgram* shaderProgram = engine->getShaderProgram(GMShaderProgramType::CurrentShaderProgram);
	shaderProgram->useProgram();

	{
		for (GMuint i = 0; i < GEOMETRY_NUM; i++)
		{
			shaderProgram->setInt(g_GBufferGeometryUniformNames[i], i);
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, d->textures[i]);
		}
	}

	constexpr GMuint GEOMETRY_OFFSET = GEOMETRY_NUM;
	{
		for (GMuint i = 0; i < MATERIAL_NUM; i++)
		{
			shaderProgram->setInt(g_GBufferMaterialUniformNames[i], GEOMETRY_OFFSET + i);
			glActiveTexture(GL_TEXTURE0 + GEOMETRY_OFFSET + i);
			glBindTexture(GL_TEXTURE_2D, d->materials[i]);
		}
	}

	shaderProgram->useProgram();
	ITexture* cubeMap = engine->getCubeMap();
	if (cubeMap)
		cubeMap->useTexture(0, GEOMETRY_OFFSET + MATERIAL_NUM);
	shaderProgram->setInt(shaderProgram->getDesc().CubeMapTextureName, GEOMETRY_OFFSET + MATERIAL_NUM);
}

void GMGLGBufferDep::copyDepthBuffer(GLuint target)
{
	D(d);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, d->fbo[(GMint)GMGLDeferredRenderState::PassingGeometry]);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target);
	glBlitFramebuffer(0, 0, d->renderWidth, d->renderHeight, 0, 0, d->renderWidth, d->renderHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

bool GMGLGBufferDep::createFrameBuffers(GMGLDeferredRenderState state, GMint textureCount, GLuint* textureArray)
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
	}

	// If using STENCIL buffer:
	// NEVER EVER MAKE A STENCIL buffer. All GPUs and all drivers do not support an independent stencil buffer.
	// If you need a stencil buffer, then you need to make a Depth=24, Stencil=8 buffer, also called D24S8.
	// See https://www.khronos.org/opengl/wiki/Framebuffer_Object_Extension_Examples

	glGenRenderbuffers(1, &d->depthBuffers[s]);
	glBindRenderbuffer(GL_RENDERBUFFER, d->depthBuffers[s]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, d->renderWidth, d->renderHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, d->depthBuffers[s]);

	if (!drawBuffers(textureCount))
		return false;

	return true;
}

bool GMGLGBufferDep::drawBuffers(GMuint count)
{
	Vector<GLuint> attachments;
	for (GMuint i = 0; i < count; i++)
	{
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	glDrawBuffers(count, attachments.data());

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		gm_error("FB incomplete error, status: 0x%x\n", status);
		return false;
	}
	return true;
}

IFramebuffers* GMGLGBuffer::createGeometryFramebuffers()
{
	for (GMint i = 0; i < GM_array_size(g_GBufferGeometryUniformNames); ++i)
	{
		IFramebuffer* framebuffer = nullptr;
		GM.getFactory()->createFramebuffer(&framebuffer);
		GM_ASSERT(framebuffer);
		/*framebuffer->init(GMFramebufferDesc);*/
	}
	return nullptr;
}

IFramebuffers* GMGLGBuffer::createMaterialFramebuffers()
{
	return nullptr;
}

void GMGLGBuffer::geometryPass(GMGameObject *objects[], GMuint count)
{
}

void GMGLGBuffer::lightPass()
{
}
