out vec4 _particle_position_world;

void particle_calcCoords()
{
    _particle_position_world = GM_WorldMatrix * position;
    gl_Position = GM_ProjectionMatrix * GM_ViewMatrix * _particle_position_world;
    _normal = normal;
    _tangent = tangent;
    _bitangent = bitangent;
    _uv = uv;
    _lightmapuv = lightmapuv;
    _color = color;
}

void GM_Particle()
{
    particle_calcCoords();
}