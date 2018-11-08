#version 330
#include "../foundation/foundation.h"
#include "../foundation/vert_header.h"

out vec4 _deferred_geometry_pass_position_world;

void deferred_geometry_pass_calcCoords()
{
    if (GM_UseBoneAnimation != 0)
    {
        mat4 boneTransform = GM_Bones[boneIDs[0]] * weights[0];
        boneTransform += GM_Bones[boneIDs[1]] * weights[1];
        boneTransform += GM_Bones[boneIDs[2]] * weights[2];
        boneTransform += GM_Bones[boneIDs[3]] * weights[3];
        position = boneTransform * position;
        normal = boneTransform * vec4(normal.xyz, 0);
    }

    gl_Position = GM_ProjectionMatrix * GM_ViewMatrix * GM_WorldMatrix * position;
    _deferred_geometry_pass_position_world = GM_WorldMatrix * position;
    _normal = normal;
    _tangent = tangent;
    _bitangent = bitangent;
    _uv = uv;
    _lightmapuv = lightmapuv;
}

void GM_GeometryPass()
{
    deferred_geometry_pass_calcCoords();
}

void main(void)
{
    init_layouts();
    GM_GeometryPass();
}