#version 330
#include "foundation/foundation.h"
#include "foundation/vert_header.h"

#include "model2d.vert"
#include "model3d.vert"
#include "particles.vert"
#include "glyph.vert"

// VERTEX
void main(void)
{
	if (GM_shader_proc == PROC_FORWARD)
	{
		switch (GM_shader_type)
		{
			case TYPE_MODEL2D:
				model2d_main();
				break;
			case TYPE_MODEL3D:
				model3d_main();
				break;
			case TYPE_PARTICLES:
				particles_main();
				break;
			case TYPE_GLYPH:
				glyph_main();
				break;
		};
	}
}