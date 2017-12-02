out vec4 _deferred_geometry_pass_shadowCoord;
out vec4 _deferred_geometry_pass_position_world;

void deferred_geometry_pass_calcCoords()
{
	gl_Position = GM_projection_matrix * GM_view_matrix * GM_model_matrix * position;
	_deferred_geometry_pass_shadowCoord = GM_shadow_matrix * GM_model_matrix * _deferred_geometry_pass_position_world;
	
	_deferred_geometry_pass_position_world = position;
	_normal = normal;
	_tangent = tangent;
	_bitangent = bitangent;
	_uv = uv;
	_lightmapuv = lightmapuv;
}

void deferred_geometry_pass_main()
{
	deferred_geometry_pass_calcCoords();
}
