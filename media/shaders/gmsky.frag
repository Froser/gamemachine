#version 330 core

uniform sampler2D GM_ambient_texture;
uniform float GM_ambient_texture_scroll_s;
uniform float GM_ambient_texture_scroll_t;
uniform int GM_ambient_texture_switch = 0;

uniform sampler2D GM_ambient_texture_2;
uniform float GM_ambient_texture_2_scroll_s;
uniform float GM_ambient_texture_2_scroll_t;
uniform int GM_ambient_texture_2_switch = 0;

uniform sampler2D GM_ambient_texture_3;
uniform float GM_ambient_texture_3_scroll_s;
uniform float GM_ambient_texture_3_scroll_t;
uniform int GM_ambient_texture_3_switch = 0;

uniform vec4 GM_light_ambient;

in vec2 _uv;
out vec4 frag_color;

void drawSky()
{
	vec3 color = GM_light_ambient * vec3(texture(GM_ambient_texture, _uv + vec2(GM_ambient_texture_scroll_s, GM_ambient_texture_scroll_t)));
    color += GM_ambient_texture_2_switch == 1 ? vec3(texture(GM_ambient_texture_2, _uv + vec2(GM_ambient_texture_2_scroll_s, GM_ambient_texture_2_scroll_t))) : vec3(0);
    color += GM_ambient_texture_3_switch == 1 ? vec3(texture(GM_ambient_texture_3, _uv + vec2(GM_ambient_texture_3_scroll_s, GM_ambient_texture_3_scroll_t))) : vec3(0);
    frag_color = vec4(color, 1.0f);
}

void main()
{
    drawSky();
}
