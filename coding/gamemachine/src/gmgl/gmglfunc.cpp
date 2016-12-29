#include "stdafx.h"
#include "gmglfunc.h"
#include "gmglshaders.h"
#include "gmgllight.h"
#include "utilities/vmath.h"
#include "utilities/camera.h"

void IMPL projection(const vmath::mat4& mat, GMGLShaders& shaders, const char* projectionMatrixName)
{
	GLuint projectionMatrixLocation = glGetUniformLocation(shaders.getProgram(), projectionMatrixName);
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, mat);
}

void IMPL frustum(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f, GMGLShaders& shaders, const char* projectionMatrixName)
{
	vmath::mat4 mat(vmath::frustum(left, right, bottom, top, n, f));
	projection(mat, shaders, projectionMatrixName);
}

void IMPL perspective(float fovy, float aspect, float n, float f, GMGLShaders& shaders, const char* projectionMatrixName)
{
	vmath::mat4 mat(vmath::perspective(fovy, aspect, n, f));
	projection(mat, shaders, projectionMatrixName);
}

void IMPL lookAt(Camera& camera, GMGLShaders& shaders, const char* viewMatrixName)
{
	CameraLookAt c = camera.getCameraLookAt();
	GLuint viewMatrixLocation = glGetUniformLocation(shaders.getProgram(), viewMatrixName);

	GMfloat vec[4] = { c.position_x, c.position_y, c.position_z, 1.0f };
	GMGLLight(shaders).setViewPosition(vec);

	vmath::mat4 view_matrix(
		vmath::lookat(vmath::vec3(c.position_x, c.position_y, c.position_z), 
			vmath::vec3(c.lookAt_x + c.position_x, c.lookAt_y + c.position_y, c.lookAt_z + c.position_z),
			vmath::vec3(0, 1, 0))
	);

	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, view_matrix);
}

void IMPL transform(GMGLShaders& shaders, GMfloat* mat, const char* transformMatrixName)
{
	GLuint projectionMatrixLocation = glGetUniformLocation(shaders.getProgram(), transformMatrixName);
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, mat);
}

void IMPL uniformMatrix4(GMGLShaders& shaders, GMfloat* mat, const char* matrixName)
{
	GLuint projectionMatrixLocation = glGetUniformLocation(shaders.getProgram(), matrixName);
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, mat);
}

void IMPL uniformTextureIndex(GMGLShaders& shaders, GMint id, const char* textureName)
{
	glUniform1i(glGetUniformLocation(shaders.getProgram(), textureName), id);
}