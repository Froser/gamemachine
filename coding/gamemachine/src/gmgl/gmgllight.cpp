#include "stdafx.h"
#include "gmgllight.h"
#include "gmglshaders.h"
#include "shader_constants.h"
#include "utilities/vmath.h"
#include "utilities/assert.h"
#include "gmglgraphic_engine.h"
#include "gmengine/elements/gameworld.h"
#include "gmdatacore/object.h"

static void setVec4(GMGLShaders& s, char* name, GMfloat* value)
{
	GLuint loc = glGetUniformLocation(s.getProgram(), name);
	CHECK_GL_LOC(loc);

	GMfloat vec[4] = { value[0], value[1], value[2], 1.0f };
	glUniform4fv(loc, 1, vec);
	ASSERT_GL();
}

static void setFloat(GMGLShaders& s, char* name, GMfloat value)
{
	GLuint loc = glGetUniformLocation(s.getProgram(), name);
	glUniform1f(loc, value);
}

void GMGLAmbientLight::activateLight(Material& material)
{
	GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(getWorld()->getGraphicEngine());
	GMGLShaders& shaders = engine->getShaders();
	setVec4(shaders, GMSHADER_LIGHT_AMBIENT, getColor());
	setVec4(shaders, GMSHADER_LIGHT_KA, material.Ka);
	setVec4(shaders, GMSHADER_LIGHT_KE, material.Ke);
}

bool GMGLAmbientLight::isAvailable()
{
	return true;
}

void GMGLSpecularLight::activateLight(Material& material)
{
	GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(getWorld()->getGraphicEngine());
	GMGLShaders& shaders = engine->getShaders();
	setVec4(shaders, GMSHADER_LIGHT_SPECULAR, getColor());
	setVec4(shaders, GMSHADER_LIGHT_KD, material.Kd);
	setVec4(shaders, GMSHADER_LIGHT_KS, material.Ks);
	setFloat(shaders, GMSHADER_LIGHT_SHININESS, material.shininess);
}

bool GMGLSpecularLight::isAvailable()
{
	return true;
}
