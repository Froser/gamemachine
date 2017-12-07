out vec3 _cubemap_uv;

void cubemap_main()
{
	_cubemap_uv = position.xyz;
	gl_Position = GM_projection_matrix * GM_view_matrix * position;
}