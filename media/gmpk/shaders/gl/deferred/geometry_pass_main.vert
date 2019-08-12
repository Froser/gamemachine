#version @@@GMGL_SHADER_VERSION@@@
#include "../foundation/foundation.h"
#include "../foundation/vert_header.h"

out vec4 _deferred_geometry_pass_position_world;

void deferred_geometry_pass_calcCoords()
{
    if (GM_UseAnimation == GM_SkeletalAnimation)
    {
        mat4 boneTransform = GM_Bones[boneIDs[0]] * weights[0];
        boneTransform += GM_Bones[boneIDs[1]] * weights[1];
        boneTransform += GM_Bones[boneIDs[2]] * weights[2];
        boneTransform += GM_Bones[boneIDs[3]] * weights[3];
        position = boneTransform * position;
        normal = boneTransform * vec4(normal.xyz, 0);
    }
    else if (GM_UseAnimation == GM_AffineAnimation)
    {
        // 将仿射变换的值放到第1个Bones矩阵中
        position = GM_Bones[0] * position;
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