#version 410
#include "../foundation/foundation.h"
#include "../foundation/vert_header.h"

// VERTEX DEFERRED
#include "../deferred/geometry_pass.vert"
#include "../deferred/material_pass.vert"

#include "geometry_pass_header.h"

void main(void)
{
	init_layouts();
	if (GM_shader_proc == PROC_GEOMETRY_PASS)
	{
		deferred_geometry_pass_main();
	}
	else if (GM_shader_proc == PROC_MATERIAL_PASS)
	{
		deferred_material_pass_main();
	}
}