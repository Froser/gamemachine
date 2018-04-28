subroutine (GM_TechniqueEntrance)
void GM_Glyph(void)
{
    gl_Position = GM_model_matrix * position;
    _uv = uv;
    _color = color;
}