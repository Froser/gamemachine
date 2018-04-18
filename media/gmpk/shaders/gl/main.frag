#version 330 core
#include "foundation/foundation.h"
#include "foundation/properties.h"
#include "foundation/frag_header.h"

// FRAGMENT
#include "model2d.frag"
#include "model3d.frag"
#include "glyph.frag"
#include "cubemap.frag"

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
		case TYPE_GLYPH:
			glyph_main();
			break;
		case TYPE_CUBEMAP:
			cubemap_main();
			break;
	};
}