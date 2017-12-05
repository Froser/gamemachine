#version 330
#include "../foundation/foundation.h"
#include "../foundation/deferred_header.h"
#include "../foundation/vert_header.h"

// VERTEX DEFERRED
#include "../deferred/geometry_pass.vert"
#include "../deferred/material_pass.vert"
#include "../deferred/light_pass.vert"

void main(void)
{
	if (GM_shader_proc == PROC_GEOMETRY_PASS)
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