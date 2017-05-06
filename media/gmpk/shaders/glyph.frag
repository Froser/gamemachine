#version 330 core

uniform sampler2D GM_ambient_texture;
uniform float GM_ambient_texture_scroll_s;
uniform float GM_ambient_texture_scroll_t;
uniform float GM_ambient_texture_scale_s;
uniform float GM_ambient_texture_scale_t;

in vec2 _uv;
out vec4 frag_color;

void main()
{
	frag_color = texture(GM_ambient_texture, 
		_uv * vec2(GM_ambient_texture_scale_s, GM_ambient_texture_scale_t) + vec2(GM_ambient_texture_scroll_s, GM_ambient_texture_scroll_t)
	);
}
