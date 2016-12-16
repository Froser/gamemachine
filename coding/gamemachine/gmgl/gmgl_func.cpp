#include "stdafx.h"
#include "gmgl_func.h"
#include "shaders.h"
#include "utilities/vmath.h"
#include "utilities/camera.h"

void projection(const vmath::mat4& mat, GMGLShaders& shaders, char* projectionMatrixName)
{
	GLuint projectionMatrixLocation = glGetUniformLocation(shaders.getProgram(), projectionMatrixName);
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, mat);
}

void IMPL frustum(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f, GMGLShaders& shaders, char* projectionMatrixName)
{
	vmath::mat4 mat(vmath::frustum(left, right, bottom, top, n, f));
	projection(mat, shaders, projectionMatrixName);
}

void IMPL perspective(float fovy, float aspect, float n, float f, GMGLShaders& shaders, char* projectionMatrixName)
{
	vmath::mat4 mat(vmath::perspective(fovy, aspect, n, f));
	projection(mat, shaders, projectionMatrixName);
}

void IMPL lookAt(Camera& camera, GMGLShaders& shaders, char* viewMatrixName)
{
	CameraLookAt c = camera.getCameraLookAt();
	GLuint viewMatrixLocation = glGetUniformLocation(shaders.getProgram(), viewMatrixName);

	vmath::mat4 view_matrix(
		vmath::lookat(vmath::vec3(c.position_x, c.position_y, c.position_z), 
			vmath::vec3(c.lookAt_x + c.position_x, c.lookAt_y + c.position_y, c.lookAt_z + c.position_z),
			vmath::vec3(0, 1, 0))
	);

	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, view_matrix);
}