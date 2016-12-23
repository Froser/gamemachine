#version 330
#define W_IS_NOT_ONE_BREAK(VEC4) if (VEC4.w - 1 > 0.001) return;
#define ZERO_BREAK(n) if (n <= 0.001) return;
#define MORE_THAN_N_BREAK(value, n) if (value > n) return;

uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform mat4 projection_matrix;

uniform vec4 light_position;
uniform vec4 view_position;
uniform float light_shininess;

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 normal;

out float diffuse;
out float specular;

void main(void)
{
	vec4 worldCoord = model_matrix * position;

    vec4 pos = projection_matrix * view_matrix * worldCoord;

    vec3 light_direction = normalize(light_position.xyz - worldCoord.xyz);

    vec3 view_direction = normalize(view_position.xyz - worldCoord.xyz);

    vec3 half_vector = 0.5 * (light_direction + view_direction);

    vec3 normalized = normal.xyz;

    diffuse = dot(light_direction, normalized);

    specular = pow(dot(half_vector, normalized), light_shininess);

    gl_Position = pos;
}