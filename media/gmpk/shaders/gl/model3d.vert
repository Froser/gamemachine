out vec4 _model3d_position_world;

void model3d_calcCoords()
{
	_model3d_position_world = GM_model_matrix * position;
	vec4 position_eye = GM_view_matrix * _model3d_position_world;
	gl_Position = GM_projection_matrix * position_eye;
	_normal = normal;
	_tangent = tangent;
	_bitangent = bitangent;
	_uv = gm_toGLTexCoord(uv);
	_lightmapuv = lightmapuv;
}

void model3d_main()
{
	model3d_calcCoords();
}