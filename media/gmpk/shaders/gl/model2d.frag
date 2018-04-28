subroutine (GM_TechniqueEntrance)
void GM_Model2D()
{
	if (GM_ambient_textures[0].enabled == 1)
	{
		_frag_color = texture(GM_ambient_textures[0].texture, _uv * vec2(GM_ambient_textures[0].scale_s, GM_ambient_textures[0].scale_t));
		if (_frag_color.a == 0)
			discard;
	}
	else
	{
		_frag_color = vec4(0, 0, 0, 0);
	}
}
