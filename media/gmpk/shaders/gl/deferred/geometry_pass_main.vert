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
	GM_techniqueEntrance();
}