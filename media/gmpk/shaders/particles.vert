#version 330

uniform mat4 GM_view_matrix;
uniform mat4 GM_projection_matrix;

layout (location = 0) in vec4 position;
layout (location = 2) in vec2 uv;
layout (location = 6) in vec4 color;

out vec2 _uv;
out vec4 _color;

void main(void)
{
    _uv = uv;
    _color = color;
    gl_Position = GM_projection_matrix * GM_view_matrix * position;
}