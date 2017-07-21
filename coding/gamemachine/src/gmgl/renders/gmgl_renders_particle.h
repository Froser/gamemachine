#ifndef __GMGL_RENDERS_PARTICLE_H__
#define __GMGL_RENDERS_PARTICLE_H__
#include "common.h"
#include "gmgl_renders_object.h"
BEGIN_NS

class GMGLRenders_Particle : public GMGLRenders_Object
{
public:
	virtual void beginShader(Shader& shader, GMDrawMode mode) override;
};

END_NS
#endif