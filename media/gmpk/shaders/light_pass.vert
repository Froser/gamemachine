#version 330

uniform mat4 GM_view_matrix;
uniform mat4 GM_model_matrix;
uniform mat4 GM_projection_matrix;

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

out vec4 _position_world;
out vec2 _texCoords;

void main()
{
    _position_world = GM_model_matrix * vec4(position, 1.0f);
    gl_Position = GM_projection_matrix * GM_view_matrix * _position_world;
    _texCoords = texCoords;
}