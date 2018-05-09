subroutine (GM_TechniqueEntrance)
void GM_Shadow()
{
	gl_Position = GM_shadowInfo.ShadowMatrix * GM_model_matrix * position;
}