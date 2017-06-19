#ifndef __GMGL_FUNC_H__
#define __GMGL_FUNC_H__
#include "common.h"
#include "foundation/linearmath.h"
BEGIN_NS

class GMGLShaderProgram;
struct CameraLookAt;
class Frustum;

struct GMGL
{
	static void projection(const linear_math::Matrix4x4& mat, GMGLShaderProgram& shaders, const char* projectionMatrixName);
	static void frustum(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f, GMGLShaderProgram& shaders, const char* projectionMatrixName);
	static void perspective(const linear_math::Matrix4x4& projectionMatrix, GMGLShaderProgram& shaders, const char* projectionMatrixName);
	static void lookAt(const linear_math::Matrix4x4& viewMatrix, GMGLShaderProgram& shaders, const char* viewMatrixName);
	static void cameraPosition(const CameraLookAt& lookAt, GMGLShaderProgram& shaders, const char* matrixName);
	static void uniformMatrix4(GMGLShaderProgram& shaders, GMfloat* mat, const char* matrixName);
	static void uniformTextureIndex(GMGLShaderProgram& shaders, GMint id, const char* textureName);
	static void uniformInt(GMGLShaderProgram& shaders, int value, const char* name);
	static void uniformVec3(GMGLShaderProgram& s, GMfloat* value, char* name);
	static void uniformFloat(GMGLShaderProgram& s, GMfloat value, char* name);
};

END_NS
#endif