out vec4 _model3d_position_world;

void model3d_calcCoords()
{
	_model3d_position_world = GM_model_matrix * position;
	vec4 position_eye = GM_view_matrix * _model3d_position_world;
	gl_Position = GM_projection_matrix * position_eye;
	_normal = normal;
	_tangent = tangent;
	_bitangent = bitangent;
	_uv = uv;
	_lightmapuv = lightmapuv;
}

subroutine (GM_TechniqueEntrance)
void GM_Model3D()
{
	model3d_calcCoords();
}