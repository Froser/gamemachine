#include "stdafx.h"
#include "gmglgbuffer.h"
#include "gmglgraphic_engine.h"
#include "foundation/gamemachine.h"

#define TEXTURE_NUM (GMuint) GBufferTextureType::EndOfTextureType
#define MATERIAL_NUM (GMuint) GBufferMaterialType::EndOfMaterialType

Array<const char*, TEXTURE_NUM> g_GBufferTextureUniformName =
{
	"gPosition",
	"gNormal",
	"gTexAmbient",
	"gTexDiffuse",
	"gTangent",
	"gBitangent",
	"gNormalMap",
};

Array<const char*, MATERIAL_NUM> g_GBufferMaterialUniformName =
{
	"gKa",
	"gKd",
	"gKs",
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
	d->currentTurn = 0;
	GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(GM.getGraphicEngine());
	engine->setRenderState(GMGLRenderState::Rendering);
}

bool GMGLGBuffer::nextPass()
{
	D(d);
	GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(GM.getGraphicEngine());
	engine->setRenderState(GMGLRenderState::PassingMaterial);
	if (++d->currentTurn == GMGLGBuffer_TotalTurn)
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
		glDeleteTextures(TEXTURE_NUM, d->textures);
		d->textures[0] = 0;
	}

	if (d->depthTexture != 0)
	{
		glDeleteTextures(1, &d->depthTexture);
		d->depthTexture = 0;
	}
}

bool GMGLGBuffer::init(GMuint windowWidth, GMuint windowHeight)
{
	D(d);
	GLenum errCode;

	d->windowWidth = windowWidth;
	d->windowHeight = windowHeight;

	// Create the FBO
	glGenFramebuffers(GMGLGBuffer_TotalTurn, d->fbo);

	// Vertex data
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d->fbo[GMGLGBuffer_Rendering]);
	glGenTextures(TEXTURE_NUM, d->textures);
	for (GMuint i = 0; i < TEXTURE_NUM; i++)
	{
		glBindTexture(GL_TEXTURE_2D, d->textures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, d->textures[i], 0);
		ASSERT((errCode = glGetError()) == GL_NO_ERROR);
	}

	// Material data
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d->fbo[GMGLGBuffer_MaterialPass]);
	glGenTextures(MATERIAL_NUM, d->materials);
	for (GMuint i = 0; i < MATERIAL_NUM; i++)
	{
		glBindTexture(GL_TEXTURE_2D, d->materials[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, d->materials[i], 0);
		ASSERT((errCode = glGetError()) == GL_NO_ERROR);
	}

	// depth
	/*
	glGenTextures(1, &d->depthTexture);
	glBindTexture(GL_TEXTURE_2D, d->depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, d->depthTexture, 0);
	ASSERT((errCode = glGetError()) == GL_NO_ERROR);
	*/

	Vector<GLenum> attachments;
	auto maxNum = max(TEXTURE_NUM, MATERIAL_NUM);
	for (GMuint i = 0; i < maxNum; i++)
	{
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	glDrawBuffers(maxNum, attachments.data());
	ASSERT((errCode = glGetError()) == GL_NO_ERROR);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		gm_error("FB error, status: 0x%x\n", status);
		return false;
	}

	// restore default FBO
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

void GMGLGBuffer::setReadBuffer(GBufferTextureType textureType)
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
	GLenum errCode;
	for (GMuint i = 0; i < TEXTURE_NUM; i++)
	{
		shaderProgram->setInt(g_GBufferTextureUniformName[i], i);
		ASSERT((errCode = glGetError()) == GL_NO_ERROR);
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, d->textures[i]);
		ASSERT((errCode = glGetError()) == GL_NO_ERROR);
	}
}

void GMGLGBuffer::activateMaterials(GMGLShaderProgram* shaderProgram)
{
	D(d);
	constexpr GMuint OFFSET = TEXTURE_NUM;
	GLenum errCode;
	for (GMuint i = 0; i < MATERIAL_NUM; i++)
	{
		shaderProgram->setInt(g_GBufferMaterialUniformName[i], OFFSET + i);
		ASSERT((errCode = glGetError()) == GL_NO_ERROR);
		glActiveTexture(GL_TEXTURE0 + OFFSET + i);
		glBindTexture(GL_TEXTURE_2D, d->materials[i]);
		ASSERT((errCode = glGetError()) == GL_NO_ERROR);
	}
}