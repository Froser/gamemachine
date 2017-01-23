#version 330

uniform mat4 GM_view_matrix;
uniform mat4 GM_model_matrix;
uniform mat4 GM_projection_matrix;
uniform vec4 GM_view_position;

layout (location = 0) in vec4 position;

out vec3 _uv;

void calcCoords()
{
    vec4 position_world = GM_model_matrix * position;
    vec4 position_eye = GM_view_matrix * position_world;
    gl_Position = GM_projection_matrix * position_eye;
    _uv = -normalize(GM_view_position.xyz - position_world.xyz);
}

void main(void)
{
    calcCoords();
}