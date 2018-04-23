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
	GM_techniqueEntrance();
}