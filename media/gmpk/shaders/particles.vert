#version 330
#include "foundation/vert_header.h"

out vec2 _uv;
out vec4 _color;

void main(void)
{
    _uv = uv;
    _color = color;
    gl_Position = GM_projection_matrix * GM_view_matrix * position;
}