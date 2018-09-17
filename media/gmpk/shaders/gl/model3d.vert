out vec4 _model3d_position_world;

void model3d_calcCoords()
{
    _model3d_position_world = GM_WorldMatrix * position;
    gl_Position = GM_ProjectionMatrix * GM_ViewMatrix * _model3d_position_world;
    _normal = normal;
    _tangent = tangent;
    _bitangent = bitangent;
    _uv = uv;
    _lightmapuv = lightmapuv;
    _color = color;
}

void GM_Model3D()
{
    model3d_calcCoords();
}