#version 330
#include "foundation/foundation.h"
#include "foundation/vert_header.h"

// VERTEX
#include "model2d.vert"
#include "model3d.vert"
#include "particles.vert"
#include "glyph.vert"

// VERTEX DEFERRED
#include "deferred/geometry_pass.vert"
#include "deferred/material_pass.vert"
#include "deferred/light_pass.vert"

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
	else if (GM_shader_proc == PROC_GEOMETRY_PASS)
	{
		deferred_geometry_pass_main();
	}
	else if (GM_shader_proc == PROC_MATERIAL_PASS)
	{
		deferred_material_pass_main();
	}
	else if (GM_shader_proc == PROC_LIGHT_PASS)
	{
		deferred_light_pass_main();
	}
}