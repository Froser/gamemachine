in vec3 _cubemap_uv;
uniform samplerCube GM_cubemap;

void cubemap_main()
{
	_frag_color = texture(GM_cubemap, _cubemap_uv);
}