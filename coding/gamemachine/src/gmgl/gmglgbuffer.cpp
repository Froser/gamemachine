#include "stdafx.h"
#include "gmglgbuffer.h"
#include "gmglgraphic_engine.h"
#include "foundation/gamemachine.h"
#include "shader_constants.h"

#define GMEngine static_cast<GMGLGraphicEngine*>(GM.getGraphicEngine())

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

bool GMGLGBuffer::init(const GMRect& clientRect)
{
	D(d);
	d->clientRect = clientRect;
	d->renderWidth = GMGetRenderState(RESOLUTION_X) == GMStates_RenderOptions::AUTO_RESOLUTION ? clientRect.width : GMGetRenderState(RESOLUTION_X);
	d->renderHeight = GMGetRenderState(RESOLUTION_Y) == GMStates_RenderOptions::AUTO_RESOLUTION ? clientRect.height : GMGetRenderState(RESOLUTION_Y);
	d->viewport = { d->clientRect.x, d->clientRect.y, d->renderWidth, d->renderHeight };

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

void GMGLGBuffer::copyDepthBuffer(GLuint target)
{
	D(d);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, d->fbo[(GMint)GMGLDeferredRenderState::PassingGeometry]);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target);
	glBlitFramebuffer(0, 0, d->renderWidth, d->renderHeight, 0, 0, d->renderWidth, d->renderHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	GM_CHECK_GL_ERROR();

	glBindFramebuffer(GL_FRAMEBUFFER, target);
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, d->renderWidth, d->renderHeight, 0, GL_RGB, GL_FLOAT, NULL);
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

static const Pair<GMEffects, const char*> s_effects_uniformNames[] =
{
	{ GMEffects::None, GMSHADER_EFFECTS_NONE },
	{ GMEffects::Inversion, GMSHADER_EFFECTS_INVERSION },
	{ GMEffects::Sharpen, GMSHADER_EFFECTS_SHARPEN },
	{ GMEffects::Blur, GMSHADER_EFFECTS_BLUR },
	{ GMEffects::Grayscale, GMSHADER_EFFECTS_GRAYSCALE },
	{ GMEffects::EdgeDetect, GMSHADER_EFFECTS_EDGEDETECT },
};

GMGLFramebuffer::~GMGLFramebuffer()
{
	disposeQuad();
	dispose();
}

void GMGLFramebuffer::dispose()
{
	D(d);
	if (d->fbo)
	{
		glDeleteFramebuffers(1, &d->fbo);
		d->fbo = 0;
	}

	if (d->texture)
	{
		glDeleteTextures(1, &d->texture);
		d->texture = 0;
	}

	if (d->depthBuffer)
	{
		glDeleteRenderbuffers(1, &d->depthBuffer);
		d->depthBuffer = 0;
	}
}

bool GMGLFramebuffer::init(const GMRect& clientRect)
{
	D(d);
	createQuad();

	GM_BEGIN_CHECK_GL_ERROR
	d->clientRect = clientRect;
	d->renderWidth = GMGetRenderState(RESOLUTION_X) == GMStates_RenderOptions::AUTO_RESOLUTION ? clientRect.width : GMGetRenderState(RESOLUTION_X);
	d->renderHeight = GMGetRenderState(RESOLUTION_Y) == GMStates_RenderOptions::AUTO_RESOLUTION ? clientRect.height : GMGetRenderState(RESOLUTION_Y);
	d->viewport = { d->clientRect.x, d->clientRect.y, d->renderWidth, d->renderHeight };

	glGenFramebuffers(1, &d->fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d->fbo);
	glGenTextures(1, &d->texture);
	glBindTexture(GL_TEXTURE_2D, d->texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, d->renderWidth, d->renderHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, d->texture, 0);

	GLuint attachments[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		gm_error("FB incomplete error, status: 0x%x\n", status);
		return false;
	}

	glGenRenderbuffers(1, &d->depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, d->depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, d->renderWidth, d->renderHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, d->depthBuffer);

	releaseBind();
	GM_END_CHECK_GL_ERROR
	return true;
}

void GMGLFramebuffer::beginDrawEffects()
{
	D(d);
	d->effects = GMGetRenderState(EFFECTS);
	if (needRenderFramebuffer())
	{
		GMEngine->setViewport(d->viewport);
		d->hasBegun = true;
		newFrame();
		bindForWriting();
	}
}

void GMGLFramebuffer::endDrawEffects()
{
	D(d);
	if (needRenderFramebuffer())
	{
		d->hasBegun = false;
		releaseBind();
	}
}

void GMGLFramebuffer::draw(GMGLShaderProgram* program)
{
	D(d);
	if (needRenderFramebuffer())
	{
		program->useProgram();
		program->setFloat(GMSHADER_EFFECTS_TEXTURE_OFFSET_X, GMGetRenderStateF(BLUR_SAMPLE_OFFSET_X) == GMStates_RenderOptions::AUTO_SAMPLE_OFFSET ? 1.f / d->renderWidth : GMGetRenderStateF(BLUR_SAMPLE_OFFSET_X));
		program->setFloat(GMSHADER_EFFECTS_TEXTURE_OFFSET_Y, GMGetRenderStateF(BLUR_SAMPLE_OFFSET_Y) == GMStates_RenderOptions::AUTO_SAMPLE_OFFSET ? 1.f / d->renderHeight : GMGetRenderStateF(BLUR_SAMPLE_OFFSET_Y));

		bindForWriting();

		if (d->effects != GMEffects::None)
		{
			GMuint eff = GMEffects::None + 1;
			while (eff != GMEffects::EndOfEffects)
			{
				if (d->effects & eff)
				{
					const char* name = useShaderProgramAndApplyEffect(program, (GMEffects)eff);
					renderQuad();
				}
				eff <<= 1;
			}
		}
		else
		{
			const char* name = useShaderProgramAndApplyEffect(program, GMEffects::None);
			renderQuad();
		}

		releaseBind();
		GMEngine->setViewport(d->clientRect);
		renderQuad();
	}
}

GLuint GMGLFramebuffer::framebuffer()
{
	D(d);
	if (needRenderFramebuffer()) 
		return d->fbo;
	return 0;
}

bool GMGLFramebuffer::needRenderFramebuffer()
{
	D(d);
	bool autoResolution = GMGetRenderState(RESOLUTION_X) == GMStates_RenderOptions::AUTO_RESOLUTION &&
		GMGetRenderState(RESOLUTION_Y) == GMStates_RenderOptions::AUTO_RESOLUTION;
	return !autoResolution || d->effects;
}

void GMGLFramebuffer::bindForWriting()
{
	D(d);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d->fbo);
}

void GMGLFramebuffer::bindForReading()
{
	D(d);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, d->fbo);
}

void GMGLFramebuffer::releaseBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GMGLFramebuffer::newFrame()
{
	bindForWriting();
	GMGLGraphicEngine::newFrameOnCurrentContext();
	releaseBind();
}

void GMGLFramebuffer::createQuad()
{
	D(d);
	if (d->quadVAO == 0)
	{
		static GLfloat quadVertices[] = {
			// Positions		// Texture Coords
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		glGenVertexArrays(1, &d->quadVAO);
		glBindVertexArray(d->quadVAO);
		glGenBuffers(1, &d->quadVBO);
		glBindBuffer(GL_ARRAY_BUFFER, d->quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
}

void GMGLFramebuffer::renderQuad()
{
	D(d);
	if (!GMEngine->isBlending())
	{
		glDisable(GL_BLEND);
	}
	else
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glBindVertexArray(d->quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	GM_CHECK_GL_ERROR();
}

void GMGLFramebuffer::disposeQuad()
{
	D(d);
	glBindVertexArray(0);
	if (d->quadVAO)
		glDeleteVertexArrays(1, &d->quadVAO);

	if (d->quadVBO)
		glDeleteBuffers(1, &d->quadVBO);
}

const char* GMGLFramebuffer::useShaderProgramAndApplyEffect(GMGLShaderProgram* program, GMEffects effect)
{
	D(d);
	const char* uniformName = nullptr;
	program->setInt("gFramebuffer", 0);
	GM_CHECK_GL_ERROR();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, d->texture);
	GM_CHECK_GL_ERROR();

	for (auto iter = std::begin(s_effects_uniformNames); iter != std::end(s_effects_uniformNames); ++iter)
	{
		if (iter->first == effect)
		{
			program->setBool(iter->second, true);
			uniformName = iter->second;
		}
		else
		{
			program->setBool(iter->second, false);
		}
	}
	return uniformName;
}