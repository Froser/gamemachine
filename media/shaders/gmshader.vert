#version 400
#define W_IS_NOT_ONE_BREAK(VEC4) if (VEC4.w - 1 > 0.001) return;
#define ZERO_BREAK(n) if (n <= 0.001) return;
#define MORE_THAN_N_BREAK(value, n) if (value > n) return;

uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform mat4 projection_matrix;
uniform mat4 shadow_matrix;

uniform vec4 light_position;
uniform vec4 view_position;
uniform float light_shininess;

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 normal;
layout (location = 2) in vec2 uv;

out float diffuse;
out float specular;
out vec4 shadow_coord;
out vec2 ambient_texture_uv;
out vec3 cubemap_texture_uv;

out vec3 normalized_view_coord;
out vec3 view_direction;

void main(void)
{
	vec4 world_coord = model_matrix * position;

    vec4 model_view_matrix = view_matrix * world_coord;

    gl_Position = projection_matrix * model_view_matrix;

    vec3 light_direction = normalize(light_position.xyz - world_coord.xyz);

    view_direction = normalize(view_position.xyz - world_coord.xyz);

    vec3 half_vector = 0.5 * (light_direction + view_direction);

    vec3 normalized_world_coord = normal.xyz;

    diffuse = dot(light_direction, normalized_world_coord);

    specular = pow(dot(half_vector, normalized_world_coord), light_shininess);

	shadow_coord = shadow_matrix * world_coord;

    ambient_texture_uv = uv;

    cubemap_texture_uv = -vec3(view_direction);

    // 这个是基于视角的法向量
    normalized_view_coord = vec3(normalize(model_view_matrix * normalized_world_coord));
}