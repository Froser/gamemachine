#ifndef __GMGL_RENDER_H__
#define __GMGL_RENDER_H__
#include "common.h"
#include "utilities/linearmath.h"

BEGIN_NS

struct IGraphicEngine;
class ChildObject;
struct Shader;
struct CameraLookAt;
struct IRender
{
	virtual ~IRender() {}
	virtual void begin(IGraphicEngine* engine, ChildObject* childObj, GMfloat* modelTransform) = 0;
	virtual void beginShader(Shader& shader) = 0;
	virtual void endShader() = 0;
	virtual void end() = 0;
	virtual void updateVPMatrices(const linear_math::Matrix4x4& projection, const linear_math::Matrix4x4& view, const CameraLookAt& lookAt) = 0;
};

END_NS
#endif