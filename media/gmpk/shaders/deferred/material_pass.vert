void deferred_material_pass_main()
{
	gl_Position = GM_projection_matrix * GM_view_matrix * GM_model_matrix * position;
}