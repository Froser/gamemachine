layout (location = 1) in vec2 deferred_light_pass_texCoords;
out vec2 _deferred_light_pass_texCoords;
out vec4 _deferred_light_pass_position_world;

void deferred_light_pass_main()
{
	_deferred_light_pass_position_world = vec4(position.xyz, 1.0f);
	gl_Position = _deferred_light_pass_position_world;
	_deferred_light_pass_texCoords = deferred_light_pass_texCoords;
}
