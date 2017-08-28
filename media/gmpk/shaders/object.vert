#include "foundation/vert_header.h"

out vec4 _shadowCoord;
out vec4 _normal;
out vec2 _uv;
out vec4 _tangent;
out vec4 _bitangent;
out vec2 _lightmapuv;
out vec4 _position_world;

void calcCoords()
{
    _position_world = GM_model_matrix * position;
    vec4 position_eye = GM_view_matrix * _position_world;
    gl_Position = GM_projection_matrix * position_eye;
    _shadowCoord = GM_shadow_matrix * _position_world;
    _normal = normal;
    _tangent = tangent;
    _bitangent = bitangent;
    _uv = uv;
    _lightmapuv = lightmapuv;
}

void main(void)
{
    calcCoords();
}