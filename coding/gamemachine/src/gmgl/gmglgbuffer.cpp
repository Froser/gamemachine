#include "stdafx.h"
#include "gmglgbuffer.h"
#include "gmglgraphic_engine.h"

#define TEXTURE_NUM (GMuint) GBufferTextureType::EndOfTextureType
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

GMGLGBuffer::GMGLGBuffer()
{
}

GMGLGBuffer::~GMGLGBuffer()
{
	dispose();
}

void GMGLGBuffer::dispose()
{
	D(d);
	if (d->fbo)
	{
		glDeleteFramebuffers(1, &d->fbo);
		d->fbo = 0;
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
	glGenFramebuffers(1, &d->fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d->fbo);

	// Create the gbuffer textures
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
	for (GMuint i = 0; i < TEXTURE_NUM; i++)
	{
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	glDrawBuffers(TEXTURE_NUM, attachments.data());
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
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d->fbo);
}

void GMGLGBuffer::bindForReading()
{
	D(d);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, d->fbo);
}

void GMGLGBuffer::releaseBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GMGLGBuffer::setReadBuffer(GBufferTextureType TextureType)
{
	glReadBuffer(GL_COLOR_ATTACHMENT0 + (GMuint) TextureType);
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