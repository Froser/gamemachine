out vec3 _cubemap_uv;

subroutine (GM_TechniqueEntrance) void GM_CubeMap()
{
	_cubemap_uv = vec3(position.x, position.y, position.z);
	gl_Position = GM_projection_matrix * GM_view_matrix * GM_model_matrix * position;
}