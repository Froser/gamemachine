#version 330 core
#include "foundation/foundation.h"
#include "foundation/properties.h"
#include "foundation/frag_header.h"

// FRAGMENT
#include "model2d.frag"
#include "model3d.frag"
#include "particles.frag"
#include "glyph.frag"

// FRAGMENT DEFERRED
#include "foundation/light_pass_frag_header.h"
#include "deferred/geometry_pass.frag"
#include "deferred/material_pass.frag"
#include "deferred/light_pass.frag"

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
	/*
	else if (GM_shader_proc == PROC_GEOMETRY_PASS)
	{
		//deferred_geometry_pass_main();
	}
	else if (GM_shader_proc == PROC_MATERIAL_PASS)
	{
		deferred_material_pass_main();
	}
	else if (GM_shader_proc == PROC_LIGHT_PASS)
	{
		deferred_light_pass_main();
	}
	*/
}