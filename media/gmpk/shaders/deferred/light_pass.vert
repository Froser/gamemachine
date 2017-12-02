out vec4 _deferred_light_pass_position_world;

void deferred_light_pass_main()
{
	_deferred_light_pass_position_world = vec4(position.xyz, 1.0f);
	gl_Position = _deferred_light_pass_position_world;
	_uv = uv;
}
