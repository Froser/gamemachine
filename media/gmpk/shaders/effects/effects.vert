#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

out vec4 _position_world;
out vec2 _texCoords;

void main()
{
    _position_world = vec4(position, 1.0f);
    gl_Position = _position_world;
    _texCoords = texCoords;
}