#version 330 core

struct GM_texture_t
{
	sampler2D texture;
	float scroll_s;
	float scroll_t;
	float scale_s;
	float scale_t;
	int enabled;
};
uniform GM_texture_t GM_normalmap_textures[1];

layout (location = 0) out float gHasNormalMap;

void main()
{
	gHasNormalMap = GM_normalmap_textures[0].enabled == 0 ? -1 : 1;
}
