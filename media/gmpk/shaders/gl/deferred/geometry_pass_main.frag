#version 410 core

#include "../foundation/foundation.h"
#include "../foundation/properties.h"
#include "../foundation/frag_header.h"

layout (location = 0) out vec4 deferred_geometry_pass_slot_0;
layout (location = 1) out vec4 deferred_geometry_pass_slot_1;
layout (location = 2) out vec4 deferred_geometry_pass_slot_2;
layout (location = 3) out vec4 deferred_geometry_pass_slot_3;
layout (location = 4) out vec4 deferred_geometry_pass_slot_4;
layout (location = 5) out vec4 deferred_geometry_pass_slot_5;
layout (location = 6) out vec4 deferred_geometry_pass_slot_6;
layout (location = 7) out vec4 deferred_geometry_pass_slot_7;

#alias deferred_geometry_pass_gPosition_Refractivity				deferred_geometry_pass_slot_0
#alias deferred_geometry_pass_gNormal								deferred_geometry_pass_slot_1
#alias deferred_geometry_pass_gTexAmbient							deferred_geometry_pass_slot_2
#alias deferred_geometry_pass_gTexDiffuse							deferred_geometry_pass_slot_3
#alias deferred_geometry_pass_gTangent_eye							deferred_geometry_pass_slot_4
#alias deferred_geometry_pass_gBitangent_eye						deferred_geometry_pass_slot_5
#alias deferred_geometry_pass_gNormalMap_bNormalMap					deferred_geometry_pass_slot_6
#alias deferred_geometry_pass_gKs_Shininess							deferred_geometry_pass_slot_7

vec4 normalToTexture(vec3 normal)
{
	return vec4((normal + 1) * .5f, 1);
}
in vec4 _deferred_geometry_pass_position_world;

vec4 deferred_geometry_pass_calcTexture(GM_texture_t textures[MAX_TEXTURE_COUNT], vec2 uv, int size)
{
	bool hasTexture = false;
	vec4 result = vec4(0);
	for (int i = 0; i < size; i++)
	{
		if (textures[i].enabled == 0)
			break;
		
		result += textures[i].enabled == 1
			? texture(textures[i].texture, uv * vec2(textures[i].scale_s, textures[i].scale_t) + vec2(textures[i].scroll_s, textures[i].scroll_t))
			: vec4(0);
		if (textures[i].enabled == 1)
			hasTexture = true;
	}

	if (!hasTexture)
		return vec4(1);

	return clamp(result, 0.0f, 1.0f);
}

void deferred_geometry_pass_calcEyeSpace()
{
	// 由顶点变换矩阵计算法向量变换矩阵
	mat3 normalEyeTransform = mat3(GM_view_matrix * GM_inverse_transpose_model_matrix);
	// normal的齐次向量最后一位必须位0，因为法线变换不考虑平移
	${deferred_geometry_pass_gNormal} = normalToTexture ( normalize( mat3(GM_inverse_transpose_model_matrix) * _normal.xyz) );

	if (GM_normalmap_textures[0].enabled == 1)
	{
		${deferred_geometry_pass_gNormalMap_bNormalMap} = texture(GM_normalmap_textures[0].texture, _uv);
		${deferred_geometry_pass_gNormalMap_bNormalMap}.a = 1;
		${deferred_geometry_pass_gTangent_eye} = normalToTexture(normalize(normalEyeTransform * _tangent.xyz).xyz);
		${deferred_geometry_pass_gBitangent_eye} = normalToTexture(normalize(normalEyeTransform * _bitangent.xyz).xyz);
	}
	else
	{
		${deferred_geometry_pass_gTangent_eye} = normalToTexture(vec3(0, 0, 0));
		${deferred_geometry_pass_gBitangent_eye} = normalToTexture(vec3(0, 0, 0));
		${deferred_geometry_pass_gNormalMap_bNormalMap} = vec4(0, 0, 0, 0);
	}
}

subroutine (GM_TechniqueEntrance)
void GM_GeometryPass()
{
	${deferred_geometry_pass_gPosition_Refractivity}.rgb = _deferred_geometry_pass_position_world.rgb;
	${deferred_geometry_pass_gPosition_Refractivity}.a = GM_material.refractivity;
	${deferred_geometry_pass_gTexAmbient} = vec4(GM_material.ka, 1) * deferred_geometry_pass_calcTexture(GM_ambient_textures, _uv, MAX_TEXTURE_COUNT) * deferred_geometry_pass_calcTexture(GM_lightmap_textures, _lightmapuv, 1);
	${deferred_geometry_pass_gTexDiffuse} = vec4(GM_material.kd, 1) * deferred_geometry_pass_calcTexture(GM_diffuse_textures, _uv, MAX_TEXTURE_COUNT);
	${deferred_geometry_pass_gKs_Shininess} = vec4(GM_material.ks * deferred_geometry_pass_calcTexture(GM_specular_textures, _uv, MAX_TEXTURE_COUNT).rgb, GM_material.shininess);

	deferred_geometry_pass_calcEyeSpace();
}

void main(void)
{
	//GM_techniqueEntrance();
	GM_GeometryPass();
}