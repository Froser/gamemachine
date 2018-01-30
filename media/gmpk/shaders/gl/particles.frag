vec4 particles_calcTexture(GM_texture_t tex, vec2 uv)
{
	bool hasTexture = false;
	vec4 result = vec4(0);
	result += tex.enabled == 1
		? texture(tex.texture, uv * vec2(tex.scale_s, tex.scale_t) + vec2(tex.scroll_s, tex.scroll_t))
		: vec4(0);
	if (tex.enabled == 1)
		hasTexture = true;

	if (!hasTexture)
		return vec4(1);

	return result;
}

void particles_main()
{
	_frag_color = particles_calcTexture(GM_ambient_textures[0], _uv) * _color;
}
