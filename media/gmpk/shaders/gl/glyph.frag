void glyph_main()
{
	if (GM_ambient_textures[0].enabled == 1)
	{
		_frag_color = texture(GM_ambient_textures[0].texture, _uv * vec2(GM_ambient_textures[0].scale_s, GM_ambient_textures[0].scale_t));
	}
	else
	{
		_frag_color = vec4(0, 0, 0, 0);
	}

	_frag_color = vec4(_color.rgb, _frag_color.r);
}
