#ifndef __GMGL_RENDERS_PARTICLE_H__
#define __GMGL_RENDERS_PARTICLE_H__
#include <gmcommon.h>
#include "gmgl_renders_3d.h"
BEGIN_NS

class GMGLRenders_Particle : public GMGLRenders_3D
{
public:
	virtual void beginShader(GMShader& shader) override;
};

END_NS
#endif