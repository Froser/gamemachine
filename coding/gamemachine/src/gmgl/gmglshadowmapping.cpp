#include "stdafx.h"
#include "gmglshadowmapping.h"
#include "utilities/vmath.h"
#include "gmglgraphic_engine.h"
#include "gmengine/elements/gameworld.h"
#include "utilities/camera.h"
#include "shader_constants.h"
#include "gmengine/elements/character.h"
#include "gmglfunc.h"

const static GMuint BUFFER_SIZE = 2048;
const static GMuint BUFFER_DEPTH = 800;

GMGLShadowMapping::GMGLShadowMapping(GMGLGraphicEngine& engine)
	: m_depthTexture(0)
	, m_frameBuffer(0)
	, m_beginDraw(false)
	, m_engine(engine)
	, m_shadowMapWidth(BUFFER_SIZE)
	, m_shadowMapHeight(BUFFER_SIZE)
{
}

GMGLShadowMapping::~GMGLShadowMapping()
{
	dispose();
}

void GMGLShadowMapping::dispose()
{
	if (m_depthTexture)
	{
		GLuint t[1] = { m_depthTexture };
		glDeleteTextures(1, t);
	}

	if (m_frameBuffer)
	{
		GLuint b[1] = { m_frameBuffer };
		glDeleteFramebuffers(1, b);
	}
}

void GMGLShadowMapping::init()
{
	GMint size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
	if (size < m_shadowMapWidth) {
		m_shadowMapWidth = size;
	}
	if (size < m_shadowMapHeight) {
		m_shadowMapHeight = size;
	}

	glGenTextures(1, &m_depthTexture);
	glBindTexture(GL_TEXTURE_2D, m_depthTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, m_shadowMapWidth, m_shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &m_frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTexture, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GMGLShadowMapping::beginDrawDepthBuffer()
{
	GLint dims[4];
	glGetIntegerv(GL_VIEWPORT, dims);
	memcpy(m_state.viewport, dims, sizeof(dims));

	m_shaders.useProgram();
	ILightController& lightCtrl = m_engine.getLightController();
	GMfloat* lightXYZ = lightCtrl.getLightPosition();
	
	m_state.lightViewMatrix = vmath::lookat(
		vmath::vec3(lightXYZ[0], lightXYZ[1], lightXYZ[2]),
		vmath::vec3(0, 0, 0),
		vmath::vec3(0, 1, 0)
	);
	GMGL::uniformMatrix4(m_shaders, m_state.lightViewMatrix, GMSHADER_VIEW_MATRIX);

	m_state.lightProjectionMatrix = vmath::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, BUFFER_DEPTH);
	GMGL::projection(m_state.lightProjectionMatrix, m_shaders, GMSHADER_PROJECTION_MATRIX);

	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	glViewport(0, 0, m_shadowMapWidth, m_shadowMapHeight);

	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.0f, 4.0f);
	m_beginDraw = true;
}

void GMGLShadowMapping::endDrawDepthBuffer()
{
	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_beginDraw = false;
}

bool GMGLShadowMapping::hasBegun()
{
	return m_beginDraw;
}

GMGLShaders& GMGLShadowMapping::getShaders()
{
	return m_shaders;
}

const GMGLShadowMapping::State& GMGLShadowMapping::getState()
{
	return m_state;
}

GMuint GMGLShadowMapping::getDepthTexture()
{
	return m_depthTexture;
}