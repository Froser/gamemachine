#version 410
#include "../foundation/foundation.h"

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
out vec2 _uv;

void main()
{
	gl_Position = vec4(position.xyz, 1);
	_uv = vec2(uv.x, 1.0f - uv.y);
}
