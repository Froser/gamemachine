#include "stdafx.h"
#include "gmglfunc.h"
#include "gmglshaders.h"
#include "utilities/vmath.h"
#include "utilities/utilities.h"
#include "utilities/assert.h"

void GMGL::projection(const vmath::mat4& mat, GMGLShaders& shaders, const char* projectionMatrixName)
{
	GLint projectionMatrixLocation = glGetUniformLocation(shaders.getProgram(), projectionMatrixName);
	CHECK_GL_LOC(projectionMatrixLocation);
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, mat);
	ASSERT_GL();
}

void GMGL::frustum(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f, GMGLShaders& shaders, const char* projectionMatrixName)
{
	vmath::mat4 mat(vmath::frustum(left, right, bottom, top, n, f));
	projection(mat, shaders, projectionMatrixName);
}

void GMGL::perspective(const vmath::mat4& projectionMatrix, GMGLShaders& shaders, const char* projectionMatrixName)
{
	projection(projectionMatrix, shaders, projectionMatrixName);
}

void GMGL::lookAt(const vmath::mat4& viewMatrix, GMGLShaders& shaders, const char* viewMatrixName)
{
	GLint viewMatrixLocation = glGetUniformLocation(shaders.getProgram(), viewMatrixName);
	ASSERT_GL();

	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, viewMatrix);
	CHECK_GL_LOC(viewMatrixLocation);
}

void GMGL::cameraPosition(const CameraLookAt& lookAt, GMGLShaders& shaders, const char* matrixName)
{
	GMfloat vec[4] = { lookAt.position[0], lookAt.position[1], lookAt.position[2], 1.0f };
	GLuint loc = glGetUniformLocation(shaders.getProgram(), matrixName);
	ASSERT_GL();
	glUniform4fv(loc, 1, vec);
	ASSERT_GL();
}

void GMGL::uniformMatrix4(GMGLShaders& shaders, GMfloat* mat, const char* matrixName)
{
	GLint loc = glGetUniformLocation(shaders.getProgram(), matrixName);
	glUniformMatrix4fv(loc, 1, GL_FALSE, mat);
}

void GMGL::uniformTextureIndex(GMGLShaders& shaders, GMint id, const char* textureName)
{
	GLint loc = glGetUniformLocation(shaders.getProgram(), textureName);
	glUniform1i(loc, id);
	char _switch[64] = "";
	strcat_s(_switch, textureName);
	strcat_s(_switch, "_switch");
	loc = glGetUniformLocation(shaders.getProgram(), _switch);
	glUniform1i(loc, 1);
}

void GMGL::uniformInt(GMGLShaders& shaders, int value, const char* name)
{
	GLint loc = glGetUniformLocation(shaders.getProgram(), name);
	glUniform1i(loc, value);
}

void GMGL::uniformVec3(GMGLShaders& s, GMfloat* value, char* name)
{
	GLuint loc = glGetUniformLocation(s.getProgram(), name);

	GMfloat vec[4] = { value[0], value[1], value[2] };
	glUniform3fv(loc, 1, vec);
}

void GMGL::uniformFloat(GMGLShaders& shaders, GMfloat value, char* name)
{
	GLint loc = glGetUniformLocation(shaders.getProgram(), name);
	glUniform1f(loc, value);
}