#version 330
#include "foundation/vert_header.h"

out vec2 _uv;

void main(void)
{
    gl_Position = position;
    _uv = uv;
}