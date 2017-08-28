#version 330 core
#include "foundation/properties.h"

in vec2 _uv;
out vec4 frag_color;

void main()
{
	frag_color = texture(GM_ambient_textures[0].texture, 
		_uv * vec2(GM_ambient_textures[0].scale_s, GM_ambient_textures[0].scale_t) + vec2(GM_ambient_textures[0].scroll_s, GM_ambient_textures[0].scroll_t)
	);
}
