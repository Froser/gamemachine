#ifndef __GMGL_RENDERS_GLYPH_H__
#define __GMGL_RENDERS_GLYPH_H__
#include "common.h"
#include "gmgl_renders_object.h"
BEGIN_NS

class GMGLRenders_Glyph : public GMGLRenders_Object
{
public:
	virtual void updateVPMatrices(const linear_math::Matrix4x4& projection, const linear_math::Matrix4x4& view, const CameraLookAt& lookAt) override;
	virtual void beginShader(Shader& shader, GMDrawMode mode) override;
};

END_NS
#endif