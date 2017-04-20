#version 330

uniform mat4 GM_view_matrix;
uniform mat4 GM_model_matrix;
uniform mat4 GM_projection_matrix;

layout (location = 0) in vec4 position;
layout (location = 2) in vec2 uv;

out vec2 _uv;

void main(void)
{
    vec4 position_world = GM_model_matrix * position;
    vec4 position_eye = GM_view_matrix * position_world;
    gl_Position = GM_projection_matrix * position_eye;
    _uv = uv;
}