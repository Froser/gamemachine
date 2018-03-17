vec2 texcoord_transform(vec2 dxTexCoord)
{
	return vec2(dxTexCoord.x, 1.f - dxTexCoord.y);
}

void glyph_main(void)
{
    gl_Position = GM_model_matrix * position;
    _uv = texcoord_transform(uv);
    _color = color;
}