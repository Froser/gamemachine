#ifndef __GMGL_RENDERS_GLYPH_H__
#define __GMGL_RENDERS_GLYPH_H__
#include <gmcommon.h>
#include "gmgl_renders_3d.h"
BEGIN_NS

class GMGLRenders_2D : public GMGLRenders_3D
{
public:
	virtual void updateVPMatrices(const glm::mat4& projection, const glm::mat4& view, const CameraLookAt& lookAt) override;
	virtual void beginShader(GMShader& shader, GMDrawMode mode) override;
};

END_NS
#endif