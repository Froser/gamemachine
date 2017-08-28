#version 330 core
#include "../foundation/properties.h"

in vec4 _position_world;
in vec4 _normal;
in vec2 _uv;
in vec4 _tangent;
in vec4 _bitangent;
in vec2 _lightmapuv;
in vec4 _shadowCoord;

uniform mat4 GM_view_matrix;
uniform mat4 GM_model_matrix;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal_eye;
layout (location = 2) out vec3 gTexAmbient;
layout (location = 3) out vec3 gTexDiffuse;
layout (location = 4) out vec3 gTangent_eye;
layout (location = 5) out vec3 gBitangent_eye;
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

mat4 removeTranslation(mat4 mat)
{
	mat4 r = mat4(
		vec4(mat[0].xyz, 0),
		vec4(mat[1].xyz, 0),
		vec4(mat[2].xyz, 0),
		vec4(0, 0, 0, 1)
	);
	return r;
}

void calcEyeSpace()
{
	// 由顶点变换矩阵计算法向量变换矩阵
	mat4 noTranslationMat = removeTranslation(GM_model_matrix);
	mat4 normalModelTransform = transpose(inverse(noTranslationMat));
	mat4 normalEyeTransform = GM_view_matrix * normalModelTransform;
	vec4 vertex_eye = GM_view_matrix * _position_world;
	// normal的齐次向量最后一位必须位0，因为法线变换不考虑平移
	gNormal_eye = normalize( (normalEyeTransform * vec4(_normal.xyz, 0)).xyz );

	if (GM_normalmap_textures[0].enabled == 1)
	{
		gTangent_eye = normalize((normalEyeTransform * vec4(_tangent.xyz, 0)).xyz);
		gBitangent_eye = normalize((normalEyeTransform * vec4(_bitangent.xyz, 0)).xyz);
	}
	else
	{
		gTangent_eye = vec3(0,0,0);
		gBitangent_eye = vec3(0,0,0);
	}
}

void main()
{
	gPosition = _position_world.xyz;
	gTexAmbient = calcTexture(GM_ambient_textures, _uv, MAX_TEXTURE_COUNT) * calcTexture(GM_lightmap_textures, _lightmapuv, 1);
	gTexDiffuse = calcTexture(GM_diffuse_textures, _uv, MAX_TEXTURE_COUNT);
	gNormalMap = texture(GM_normalmap_textures[0].texture, _uv).rgb;
	calcEyeSpace();
}
