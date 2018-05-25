subroutine (GM_TechniqueEntrance)
void GM_Text(void)
{
    if (GM_AmbientTextureAttribute.Enabled == 1)
    {
        _frag_color = texture(GM_AmbientTextureAttribute.Texture, _uv * vec2(GM_AmbientTextureAttribute.ScaleX, GM_AmbientTextureAttribute.ScaleY));
    }
    else
    {
        _frag_color = vec4(0, 0, 0, 0);
    }

    _frag_color = vec4(_color.rgb, _frag_color.r);
}
