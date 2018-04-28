subroutine (GM_TechniqueEntrance)
void GM_Model2D()
{
	gl_Position = GM_model_matrix * position;
	_uv = uv;
}