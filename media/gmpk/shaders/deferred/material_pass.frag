#version 330 core

struct GM_Material_t
{
	vec3 ka;
	vec3 kd;
	vec3 ks;
	float shininess;
};
uniform GM_Material_t GM_material;

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

layout (location = 0) out vec3 gKa;
layout (location = 1) out vec3 gKd;
layout (location = 2) out vec3 gKs;
layout (location = 3) out vec3 gShininess;
layout (location = 4) out vec3 gHasNormalMap;

void main()
{
	gKa = GM_material.ka;
	gKd = GM_material.kd;
	gKs = GM_material.ks;
	gShininess = vec3(GM_material.shininess, 0, 0);
	gHasNormalMap = GM_normalmap_textures[0].enabled == 0 ? vec3(-1, 0, 0) : vec3(1, 0, 0);
}
