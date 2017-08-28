#version 330 core
#include "foundation/properties.h"

in vec2 _uv;
in vec4 _color;

uniform GM_texture_t GM_ambient_texture;

out vec4 frag_color;

vec4 calcTexture(GM_texture_t tex, vec2 uv)
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

void main()
{
	frag_color = calcTexture(GM_ambient_texture, _uv) * _color;
}
