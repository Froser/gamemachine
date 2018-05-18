subroutine (GM_TechniqueEntrance)
void GM_Glyph(void)
{
    gl_Position = GM_WorldMatrix * position;
    _uv = uv;
    _color = color;
}