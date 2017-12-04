#ifndef __GMGL_RENDERS_GLYPH_H__
#define __GMGL_RENDERS_GLYPH_H__
#include <gmcommon.h>
#include "gmgl_render_3d.h"
BEGIN_NS

class GMGLRender_2D : public GMGLRender_3D
{
public:
	virtual void beginComponent(GMComponent* component) override;
};

END_NS
#endif