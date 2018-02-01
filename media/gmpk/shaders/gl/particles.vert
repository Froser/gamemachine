void particles_main()
{
	_uv = gm_toGLTexCoord(uv);
	_color = color;
	gl_Position = GM_projection_matrix * GM_view_matrix * position;
}