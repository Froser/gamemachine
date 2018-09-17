#version 330
#include "../foundation/foundation.h"
#include "../foundation/vert_header.h"

out vec4 _deferred_geometry_pass_position_world;

void deferred_geometry_pass_calcCoords()
{
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