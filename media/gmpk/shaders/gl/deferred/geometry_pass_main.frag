#version 410 core
#include "../foundation/foundation.h"
#include "../foundation/properties.h"
#include "../foundation/frag_header.h"

#include "geometry_pass_header.h"
#include "geometry_pass_frag_header.h"

// FRAGMENT DEFERRED
#include "../deferred/geometry_pass.frag"
#include "../deferred/material_pass.frag"

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
}