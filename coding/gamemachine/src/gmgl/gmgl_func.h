#ifndef __GMGL_FUNC_H__
#define __GMGL_FUNC_H__
#include "common.h"
BEGIN_NS

class GMGLShaders;
class Camera;

#define BEGIN_GMGL_FUNCS class GMGL { public:
#define END_GMGL_FUNCS };
#define API static
#define IMPL GMGL::

BEGIN_GMGL_FUNCS

void API frustum(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f, GMGLShaders& shaders, char* projectionMatrixName);
void API perspective(float fovy, float aspect, float n, float f, GMGLShaders& shaders, char* projectionMatrixName);
void API lookAt(Camera& camera, GMGLShaders& shaders, char* viewMatrixName);
void API ambient(GMfloat* rgb, GMGLShaders& shaders, char* ambientMatrixName);

END_GMGL_FUNCS

END_NS
#endif