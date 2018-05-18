subroutine (GM_TechniqueEntrance)
void GM_Model2D()
{
    if (GM_AmbientTextureAttribute.Enabled == 1)
    {
        _frag_color = texture(GM_AmbientTextureAttribute.Texture, _uv * vec2(GM_AmbientTextureAttribute.ScaleX, GM_AmbientTextureAttribute.ScaleY));
        if (_frag_color.a == 0)
            discard;
    }
    else
    {
        _frag_color = vec4(0, 0, 0, 0);
    }
}
