out vec4 _model3d_position_world;

void model3d_calcCoords()
{
	_model3d_position_world = GM_model_matrix * position;
	gl_Position = GM_projection_matrix * GM_view_matrix * _model3d_position_world;
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