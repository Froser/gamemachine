subroutine (GM_TechniqueEntrance)
void GM_Shadow()
{
    gl_Position = GM_ShadowInfo.ShadowMatrix * GM_WorldMatrix * position;
}