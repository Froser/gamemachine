
layout (location = 0) in vec3 gm_position;
layout (location = 1) in vec3 gm_normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 gm_tangent;
layout (location = 4) in vec3 gm_bitangent;
layout (location = 5) in vec2 lightmapuv;
layout (location = 6) in vec4 color;
layout (location = 7) in ivec4 boneIDs;
layout (location = 8) in vec4 weights;

out vec4 _position;
out vec4 _normal;
out vec2 _uv;
out vec4 _tangent;
out vec4 _bitangent;
out vec2 _lightmapuv;
out vec4 _color;

vec4 position;
vec4 normal;
vec4 tangent;
vec4 bitangent;

void init_layouts()
{
    position = vec4(gm_position.xyz, 1);
    normal = vec4(gm_normal.xyz, 1);
    tangent = vec4(gm_tangent.xyz, 1);
    bitangent = vec4(gm_bitangent.xyz, 1);
}