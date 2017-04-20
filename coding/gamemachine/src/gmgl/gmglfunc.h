#ifndef __GMGL_FUNC_H__
#define __GMGL_FUNC_H__
#include "common.h"
#include "utilities/vmath.h"
BEGIN_NS

class GMGLShaders;
struct CameraLookAt;
class Frustum;

struct GMGL
{
	static void projection(const vmath::mat4& mat, GMGLShaders& shaders, const char* projectionMatrixName);
	static void frustum(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f, GMGLShaders& shaders, const char* projectionMatrixName);
	static void perspective(vmath::mat4 projectionMatrix, GMGLShaders& shaders, const char* projectionMatrixName);
	static void lookAt(vmath::mat4& viewMatrix, GMGLShaders& shaders, const char* viewMatrixName);
	static void cameraPosition(const CameraLookAt& lookAt, GMGLShaders& shaders, const char* matrixName);
	static void uniformMatrix4(GMGLShaders& shaders, GMfloat* mat, const char* matrixName);
	static void uniformTextureIndex(GMGLShaders& shaders, GMint id, const char* textureName);
	static void uniformInt(GMGLShaders& shaders, int value, const char* name);
};

END_NS
#endif