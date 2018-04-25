#version 410
#include "../foundation/foundation.h"
#include "../foundation/vert_header.h"

void main()
{
	init_layouts();
	gl_Position = position;
	_uv = vec2(uv.x, 1.0f - uv.y);
}
