#version 330
#include "foundation/foundation.h"
#include "foundation/vert_header.h"

// VERTEX
#include "model2d.vert"
#include "model3d.vert"
#include "particles.vert"
#include "glyph.vert"
#include "cubemap.vert"

void main(void)
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
		case TYPE_CUBEMAP:
			cubemap_main();
			break;
	};
}