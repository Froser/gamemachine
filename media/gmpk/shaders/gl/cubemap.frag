in vec3 _cubemap_uv;

void cubemap_main()
{
	_frag_color = texture(GM_cubemap_texture, _cubemap_uv);
}