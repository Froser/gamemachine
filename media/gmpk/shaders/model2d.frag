#version 330 core
#include "foundation/properties.h"

in vec2 _uv;
out vec4 frag_color;

void main()
{
	if (GM_ambient_textures[0].enabled == 1)
	{
		frag_color = texture(GM_ambient_textures[0].texture, _uv * vec2(GM_ambient_textures[0].scale_s, GM_ambient_textures[0].scale_t));
		//frag_color = vec4((frag_color.rgb - vec3(1, 1, 1)) * frag_color.a, 1);
	}
	else
	{
		frag_color = vec4(0, 0, 0, 0);
	}
}
