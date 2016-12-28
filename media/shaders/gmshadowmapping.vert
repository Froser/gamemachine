#version 400

uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform mat4 projection_matrix;

layout (location = 0) in vec4 position;

void main()
{
    gl_Position = projection_matrix * view_matrix * model_matrix * position;
}
