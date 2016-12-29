#include "stdafx.h"
#include "gmgllight.h"
#include "gmglshaders.h"
#include "shader_constants.h"
#include "utilities/vmath.h"

static void setVec4(GMGLShaders& s, char* name, GMfloat* value)
{
	GLuint loc = glGetUniformLocation(s.getProgram(), name);
	GMfloat vec[4] = { value[0], value[1], value[2], 1.0f };
	glUniform4fv(loc, 1, vec);
}

static void setFloat(GMGLShaders& s, char* name, GMfloat value)
{
	GLuint loc = glGetUniformLocation(s.getProgram(), name);
	glUniform1f(loc, value);
}

GMGLLight::GMGLLight(GMGLShaders& shaders)
	: m_shaders(shaders)
{
}

void GMGLLight::setAmbient(GMfloat rgb[3])
{
	setVec4(m_shaders, GMSHADER_LIGHT_AMBIENT, rgb);
}

void GMGLLight::setAmbientCoefficient(GMfloat ka[3])
{
	setVec4(m_shaders, GMSHADER_LIGHT_KA, ka);
}

void GMGLLight::setLightColor(GMfloat rgb[3])
{
	setVec4(m_shaders, GMSHADER_LIGHT_COLOR, rgb);
}

void GMGLLight::setDiffuseCoefficient(GMfloat kd[3])
{
	setVec4(m_shaders, GMSHADER_LIGHT_KD, kd);
}

void GMGLLight::setShininess(GMfloat n)
{
	setFloat(m_shaders, GMSHADER_LIGHT_SHININESS, n);
}

void GMGLLight::setSpecularCoefficient(GMfloat ks[3])
{
	setVec4(m_shaders, GMSHADER_LIGHT_KS, ks);
}

void GMGLLight::setEnvironmentCoefficient(GMfloat ke[3])
{
	setVec4(m_shaders, GMSHADER_LIGHT_KE, ke);
}

void GMGLLight::setLightPosition(GMfloat xyz[3])
{
	setVec4(m_shaders, GMSHADER_LIGHT_POSITION, xyz);
	memcpy(m_lightPosition, xyz, sizeof(m_lightPosition));
}

GMfloat* GMGLLight::getLightPosition()
{
	return m_lightPosition;
}

void GMGLLight::setViewPosition(GMfloat xyz[3])
{
	setVec4(m_shaders, GMSHADER_VIEW_POSITION, xyz);
}