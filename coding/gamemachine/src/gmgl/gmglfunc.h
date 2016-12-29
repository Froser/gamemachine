#ifndef __GMGL_FUNC_H__
#define __GMGL_FUNC_H__
#include "common.h"
#include "utilities/vmath.h"
BEGIN_NS

class GMGLShaders;
class Camera;

#define BEGIN_GMGL_FUNCS class GMGL { public:
#define END_GMGL_FUNCS };
#define API static
#define IMPL GMGL::

BEGIN_GMGL_FUNCS

void API projection(const vmath::mat4& mat, GMGLShaders& shaders, const char* projectionMatrixName);
void API frustum(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f, GMGLShaders& shaders, const char* projectionMatrixName);
void API perspective(float fovy, float aspect, float n, float f, GMGLShaders& shaders, const char* projectionMatrixName);
void API lookAt(Camera& camera, GMGLShaders& shaders, const char* viewMatrixName);
void API transform(GMGLShaders& shaders, GMfloat* mat, const char* transformMatrixName);
void API uniformMatrix4(GMGLShaders& shaders, GMfloat* mat, const char* matrixName);
void API uniformTextureIndex(GMGLShaders& shaders, GMint id, const char* textureName);

END_GMGL_FUNCS

END_NS
#endif