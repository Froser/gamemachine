#version 330
#include "foundation/vert_header.h"

out vec2 _uv;
out vec4 _color;

void main(void)
{
    gl_Position = GM_model_matrix * position;
    _uv = uv;
    _color = color;
}