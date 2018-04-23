subroutine (GM_TechniqueEntrance) void GM_MaterialPass()
{
	gl_Position = GM_projection_matrix * GM_view_matrix * GM_model_matrix * position;
}