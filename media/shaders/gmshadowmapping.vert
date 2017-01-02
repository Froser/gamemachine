#version 400

uniform mat4 GM_view_matrix;
uniform mat4 GM_model_matrix;
uniform mat4 GM_projection_matrix;

layout (location = 0) in vec4 position;

void main()
{
    gl_Position = GM_projection_matrix * GM_view_matrix * GM_model_matrix * position;
}
