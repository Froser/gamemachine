#include "../foundation/vert_header.h"

out vec4 _shadowCoord;
out vec4 _normal;
out vec2 _uv;
out vec4 _tangent;
out vec4 _bitangent;
out vec2 _lightmapuv;
out vec4 _position_world;

void calcCoords()
{
    gl_Position = GM_projection_matrix * GM_view_matrix * GM_model_matrix * position;
    _shadowCoord = GM_shadow_matrix * GM_model_matrix * _position_world;
    
    _position_world = position;
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