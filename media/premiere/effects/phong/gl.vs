#version 330
layout (location = 0) in vec3 gm_position;
layout (location = 1) in vec3 gm_normal;

vec4 position;
vec4 normal;
out vec4 _position;
out vec4 _normal;

uniform mat4 GM_ViewMatrix;
uniform mat4 GM_WorldMatrix;
uniform mat4 GM_ProjectionMatrix;

void init_layouts()
{
    position = vec4(gm_position.xyz, 1);
    normal = normalize(vec4(mat3(GM_WorldMatrix) * gm_normal.xyz, 1));
}

void main()
{
    init_layouts();

    gl_Position = GM_ProjectionMatrix * GM_ViewMatrix * GM_WorldMatrix * position;
    _position = position;
    _normal = normal;
}