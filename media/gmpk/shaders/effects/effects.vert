#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

out vec2 _texCoords;
void main()
{
	gl_Position = vec4(position, 1.0f);
	_texCoords = texCoords;
}
