subroutine (GM_TechniqueEntrance)
void GM_Shadow()
{
	gl_Position = GM_shadowInfo.ShadowProjectionMatrix * GM_shadowInfo.ShadowViewMatrix * GM_model_matrix * position;
}