out vec3 _cubemap_uv;

void GM_CubeMap()
{
    _cubemap_uv = vec3(position.x, position.y, position.z);
    gl_Position = GM_ProjectionMatrix * GM_ViewMatrix * GM_WorldMatrix * position;
}