#version 330 core

in vec2 _uv;
in vec4 _color;

#define MAX_TEXTURE_COUNT 3
struct GM_texture_t
{
	sampler2D texture;
	float scroll_s;
	float scroll_t;
	float scale_s;
	float scale_t;
	int enabled;
};
uniform GM_texture_t GM_ambient_textures[MAX_TEXTURE_COUNT];

out vec4 frag_color;

vec4 calcTexture(GM_texture_t textures[MAX_TEXTURE_COUNT], vec2 uv)
{
	bool hasTexture = false;
	vec4 result = vec4(0);
	result += textures[0].enabled == 1
		? texture(textures[0].texture, uv * vec2(textures[0].scale_s, textures[0].scale_t) + vec2(textures[0].scroll_s, textures[0].scroll_t))
		: vec4(0);
	if (textures[0].enabled == 1)
		hasTexture = true;

	if (!hasTexture)
		return vec4(1);

	return result;
}

void calcColor()
{
	// 最终结果
	vec4 ambientTextureColor = calcTexture(GM_ambient_textures, _uv);
	frag_color = ambientTextureColor * _color;
}

void main()
{
	calcColor();
}
