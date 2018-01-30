void model2d_main()
{
	gl_Position = GM_model_matrix * position;
	_uv = uv;
}