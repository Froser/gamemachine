﻿#ifndef __GMGL_RENDERS_LIGHTPASS_H__
#define __GMGL_RENDERS_LIGHTPASS_H__
#include <gmcommon.h>
#include <shader.h>
#include <gmmodel.h>
BEGIN_NS

class GMGLGraphicEngine;
GM_PRIVATE_OBJECT(GMGLRenders_LightPass)
{
	GMGLGraphicEngine* engine;
};

class Shader;
class GMGLRenders_LightPass : public GMObject, public IRender
{
	DECLARE_PRIVATE(GMGLRenders_LightPass)

public:
	GMGLRenders_LightPass();

public:
	virtual void begin(IGraphicEngine* engine, GMMesh* mesh, GMfloat* modelTransform) override {}
	virtual void beginShader(Shader& shader, GMDrawMode mode) override { ASSERT(false); }
	virtual void updateVPMatrices(const linear_math::Matrix4x4& projection, const linear_math::Matrix4x4& view, const CameraLookAt& lookAt);
	virtual void endShader() { ASSERT(false); }
	virtual void end() {}
	virtual void activateLights(const GMLight* lights, GMint count);
};

END_NS
#endif