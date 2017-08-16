#version 330 core

in vec2 _uv;
in vec4 _color;

struct GM_texture_t
{
	sampler2D texture;
	float scroll_s;
	float scroll_t;
	float scale_s;
	float scale_t;
	int enabled;
};
uniform GM_texture_t GM_ambient_texture;

out vec4 frag_color;

vec4 calcTexture(GM_texture_t texture, vec2 uv)
{
	bool hasTexture = false;
	vec4 result = vec4(0);
	result += texture.enabled == 1
		? texture(texture.texture, uv * vec2(texture.scale_s, texture.scale_t) + vec2(texture.scroll_s, texture.scroll_t))
		: vec4(0);
	if (texture.enabled == 1)
		hasTexture = true;

	if (!hasTexture)
		return vec4(1);

	return result;
}

void main()
{
	frag_color = calcTexture(GM_ambient_texture, _uv) * _color;
}
