void GM_Model2D()
{
    if (GM_ColorVertexOp == GM_VertexColorOp_Replace)
    {
        _frag_color = _color;
    }
    else
    {
        _frag_color = vec4(0, 0, 0, 0);
        if (GM_AmbientTextureAttribute.Enabled == 0 && GM_AmbientTextureAttribute.Enabled == 0)
            return;

        if (GM_AmbientTextureAttribute.Enabled != 0)
            _frag_color += texture(GM_AmbientTextureAttribute.Texture, _uv * vec2(GM_AmbientTextureAttribute.ScaleX, GM_AmbientTextureAttribute.ScaleY));

        if (GM_DiffuseTextureAttribute.Enabled != 0)
            _frag_color += texture(GM_DiffuseTextureAttribute.Texture, _uv * vec2(GM_DiffuseTextureAttribute.ScaleX, GM_DiffuseTextureAttribute.ScaleY));

        if (GM_ColorVertexOp == GM_VertexColorOp_NoColor)
            ;// do nothing
        if (GM_ColorVertexOp == GM_VertexColorOp_Add)
            _frag_color += _color;
        else
            _frag_color *= _color;
    }
}
