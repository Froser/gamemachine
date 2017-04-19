#include "stdafx.h"
#include "gmglfunc.h"
#include "gmglshaders.h"
#include "utilities/vmath.h"
#include "utilities/camera.h"
#include "utilities/assert.h"
#include "utilities/frustum.h"

void IMPL projection(const vmath::mat4& mat, GMGLShaders& shaders, const char* projectionMatrixName)
{
	GLint projectionMatrixLocation = glGetUniformLocation(shaders.getProgram(), projectionMatrixName);
	CHECK_GL_LOC(projectionMatrixLocation);
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, mat);
}

void IMPL frustum(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f, GMGLShaders& shaders, const char* projectionMatrixName)
{
	vmath::mat4 mat(vmath::frustum(left, right, bottom, top, n, f));
	projection(mat, shaders, projectionMatrixName);
}

void IMPL perspective(vmath::mat4 projectionMatrix, GMGLShaders& shaders, const char* projectionMatrixName)
{
	projection(projectionMatrix, shaders, projectionMatrixName);
}

void IMPL lookAt(vmath::mat4& viewMatrix, GMGLShaders& shaders, const char* viewMatrixName)
{
	GLint viewMatrixLocation = glGetUniformLocation(shaders.getProgram(), viewMatrixName);
	ASSERT_GL();

	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, viewMatrix);
	CHECK_GL_LOC(viewMatrixLocation);
}

void IMPL cameraPosition(const CameraLookAt& lookAt, GMGLShaders& shaders, const char* matrixName)
{
	GMfloat vec[4] = { lookAt.position[0], lookAt.position[1], lookAt.position[2], 1.0f };
	GLuint loc = glGetUniformLocation(shaders.getProgram(), matrixName);
	ASSERT_GL();
	glUniform4fv(loc, 1, vec);
	ASSERT_GL();
}

void IMPL uniformMatrix4(GMGLShaders& shaders, GMfloat* mat, const char* matrixName)
{
	GLint loc = glGetUniformLocation(shaders.getProgram(), matrixName);
	glUniformMatrix4fv(loc, 1, GL_FALSE, mat);
}

void IMPL uniformTextureIndex(GMGLShaders& shaders, GMint id, const char* textureName)
{
	GLint loc = glGetUniformLocation(shaders.getProgram(), textureName);
	glUniform1i(loc, id);
	char _switch[64] = "";
	strcat_s(_switch, textureName);
	strcat_s(_switch, "_switch");
	loc = glGetUniformLocation(shaders.getProgram(), _switch);
	glUniform1i(loc, 1);
}

void IMPL uniformInt(GMGLShaders& shaders, int value, const char* name)
{
	GLint loc = glGetUniformLocation(shaders.getProgram(), name);
	ASSERT_GL();
	glUniform1i(loc, value);
	ASSERT_GL();
}