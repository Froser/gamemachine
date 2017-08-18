#version 330 core

in vec4 _position_world;
in vec4 _normal;
in vec2 _uv;
in vec4 _tangent;
in vec4 _bitangent;
in vec2 _lightmapuv;
in vec4 _shadowCoord;

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
uniform GM_texture_t GM_diffuse_textures[MAX_TEXTURE_COUNT];
uniform GM_texture_t GM_lightmap_textures[MAX_TEXTURE_COUNT];  // 用到的只有1个
uniform GM_texture_t GM_normalmap_textures[1];

out vec4 frag_color;

layout (location = 0) out vec3 gWorld;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gTexAmbient;
layout (location = 3) out vec3 gTexDiffuse;
layout (location = 4) out vec3 gTangent;
layout (location = 5) out vec3 gBitangent;
layout (location = 6) out vec3 gNormalMap;

vec3 calcTexture(GM_texture_t textures[MAX_TEXTURE_COUNT], vec2 uv, int size)
{
	bool hasTexture = false;
	vec3 result = vec3(0);
	for (int i = 0; i < size; i++)
	{
		result += textures[i].enabled == 1
			? vec3(texture(textures[i].texture, uv * vec2(textures[i].scale_s, textures[i].scale_t) + vec2(textures[i].scroll_s, textures[i].scroll_t)))
			: vec3(0);
		if (textures[i].enabled == 1)
			hasTexture = true;
	}

	if (!hasTexture)
		return vec3(1);

	return result;
}

void main()
{
	gWorld = _position_world.xyz;
	gNormal = _normal.xyz;
	gTexAmbient = calcTexture(GM_ambient_textures, _uv, MAX_TEXTURE_COUNT) * calcTexture(GM_lightmap_textures, _lightmapuv, 1);
	gTexDiffuse = calcTexture(GM_diffuse_textures, _uv, MAX_TEXTURE_COUNT);
	gTangent = _tangent.xyz;
	gBitangent = _bitangent.xyz;
	gNormalMap = texture(GM_normalmap_textures[0].texture, _uv).rgb;
}
