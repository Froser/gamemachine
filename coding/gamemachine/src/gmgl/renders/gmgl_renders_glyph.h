#ifndef __GMGL_RENDERS_GLYPH_H__
#define __GMGL_RENDERS_GLYPH_H__
#include "common.h"
#include "gmgl_renders_object.h"
BEGIN_NS

class GMGLRenders_Glyph : public GMGLRenders_Object
{
public:
	virtual void updateVPMatrices(const vmath::mat4& projection, const vmath::mat4& view, const CameraLookAt& lookAt) override;
	virtual void beginShader(Shader& shader) override;
};

END_NS
#endif