void GM_Shadow()
{
    gl_Position = GM_ShadowInfo.ShadowMatrix[GM_ShadowInfo.CurrentCascadeLevel] * GM_WorldMatrix * position;
}