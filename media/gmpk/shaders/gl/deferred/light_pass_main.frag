#version 330
#include "../foundation/foundation.h"
#include "../foundation/properties.h"

in vec2 _uv;
out vec4 _frag_color;

uniform sampler2D deferred_light_pass_gPosition;
uniform sampler2D deferred_light_pass_gNormal;
uniform sampler2D deferred_light_pass_gNormal_eye;
uniform sampler2D deferred_light_pass_gTexAmbient;
uniform sampler2D deferred_light_pass_gTexDiffuse;
uniform sampler2D deferred_light_pass_gTangent_eye;
uniform sampler2D deferred_light_pass_gBitangent_eye;
uniform sampler2D deferred_light_pass_gNormalMap;

uniform sampler2D deferred_material_pass_gKa;
uniform sampler2D deferred_material_pass_gKd;
uniform sampler2D deferred_material_pass_gKs_gShininess;
uniform sampler2D deferred_material_pass_gHasNormalMap_gRefractivity;

// geometries
vec3 deferred_light_pass_fromTexture_Position;
vec3 deferred_light_pass_fromTexture_Normal;
vec3 deferred_light_pass_fromTexture_Normal_eye;
vec3 deferred_light_pass_fromTexture_TexAmbient;
vec3 deferred_light_pass_fromTexture_TexDiffuse;
vec3 deferred_light_pass_fromTexture_Tangent_eye;
vec3 deferred_light_pass_fromTexture_Bitangent_eye;
vec3 deferred_light_pass_fromTexture_NormalMap;

// materials
vec3 deferred_material_pass_fromTexture_Ka;
vec3 deferred_material_pass_fromTexture_Kd;
vec3 deferred_material_pass_fromTexture_Ks;
float deferred_material_pass_fromTexture_Shininess;

// flags, -1 or 1
float deferred_material_pass_fromTexture_HasNormalMap = -1;

// refractivity
float deferred_material_pass_fromTexture_Refractivity = 0;

// 全局变量：
// 相机视角法向量
vec3 deferred_light_pass_g_normal_eye;
// ShadowMap的阴影系数，如果没有ShadowMap则为1
vec3 deferred_light_pass_g_ambientLight;
vec3 deferred_light_pass_g_diffuseLight;
vec3 deferred_light_pass_g_specularLight;
vec3 deferred_light_pass_g_refractiveLight;

// [0, 1] -> [-1, 1]
vec3 textureToNormal(vec3 tex)
{
	return tex * 2.f - 1;
}

bool hasFlag(float flag)
{
	return flag > 0;
}

void deferred_light_pass_init()
{
	// light pass
	deferred_light_pass_g_ambientLight = deferred_light_pass_g_diffuseLight = deferred_light_pass_g_specularLight = vec3(0);
	deferred_light_pass_fromTexture_Position = texture(deferred_light_pass_gPosition, _uv).rgb;
	deferred_light_pass_fromTexture_Normal = textureToNormal(texture(deferred_light_pass_gNormal, _uv).rgb);
	deferred_light_pass_fromTexture_Normal_eye = textureToNormal(texture(deferred_light_pass_gNormal_eye, _uv).rgb);
	deferred_light_pass_fromTexture_TexAmbient = texture(deferred_light_pass_gTexAmbient, _uv).rgb;
	deferred_light_pass_fromTexture_TexDiffuse = texture(deferred_light_pass_gTexDiffuse, _uv).rgb;
	deferred_light_pass_fromTexture_Tangent_eye = texture(deferred_light_pass_gTangent_eye, _uv).rgb;
	deferred_light_pass_fromTexture_Bitangent_eye = texture(deferred_light_pass_gBitangent_eye, _uv).rgb;
	deferred_light_pass_fromTexture_NormalMap = texture(deferred_light_pass_gNormalMap, _uv).rgb;

	// material pass
	deferred_material_pass_fromTexture_Ka = texture(deferred_material_pass_gKa, _uv).rgb;
	deferred_material_pass_fromTexture_Kd = texture(deferred_material_pass_gKd, _uv).rgb;

	vec4 tKs_tShininess = texture(deferred_material_pass_gKs_gShininess, _uv);
	deferred_material_pass_fromTexture_Ks = tKs_tShininess.rgb;
	deferred_material_pass_fromTexture_Shininess = tKs_tShininess.a;

	vec4 fromTexture_HasNormalMap_tRefractivity = texture(deferred_material_pass_gHasNormalMap_gRefractivity, _uv);
	deferred_material_pass_fromTexture_HasNormalMap = fromTexture_HasNormalMap_tRefractivity.r;
	deferred_material_pass_fromTexture_Refractivity = fromTexture_HasNormalMap_tRefractivity.g;
}

void deferred_light_pass_calcDiffuseAndSpecular(GM_light_t light, vec3 lightDirection, vec3 eyeDirection, vec3 normal)
{
	vec3 N = normalize(normal);
	vec3 L = normalize(lightDirection);

	//diffuse:
	{
		float diffuseFactor = dot(L, N);
		diffuseFactor = clamp(diffuseFactor, 0.0f, 1.0f);

		deferred_light_pass_g_diffuseLight += diffuseFactor * deferred_material_pass_fromTexture_Kd * light.lightColor;
	}

	// specular:
	{
		vec3 V = normalize(eyeDirection);
		vec3 R = reflect(-L, N);
		float theta = dot(V, R);
		float specularFactor = pow(theta, deferred_material_pass_fromTexture_Shininess);
		specularFactor = clamp(specularFactor, 0.0f, 1.0f);

		deferred_light_pass_g_specularLight += specularFactor * deferred_material_pass_fromTexture_Ks * light.lightColor;
	}
}

void model3d_calculateRefractionByNormalWorld(vec3 normal_world)
{
	vec3 I = normalize(deferred_light_pass_fromTexture_Position - GM_view_position.rgb);
	vec3 R = refract(I, normal_world, deferred_material_pass_fromTexture_Refractivity);
	deferred_light_pass_g_refractiveLight += texture(GM_cubemap_texture, vec3(R.x, R.y, R.z)).rgb;
}

void model3d_calculateRefractionByNormalTangent(mat3 TBN, vec3 normal_tangent)
{
	// 如果是切线空间，计算会复杂点，要将切线空间的法线换算回世界空间
	vec3 normal_world = (GM_inverse_view_matrix * vec4(transpose(TBN) * normal_tangent, 0)).rgb;
	model3d_calculateRefractionByNormalWorld(normal_world);
}

void deferred_light_pass_calcLights()
{
	// 由顶点变换矩阵计算法向量变换矩阵
	vec4 vertex_eye = GM_view_matrix * vec4(deferred_light_pass_fromTexture_Position, 1);
	vec3 eyeDirection_eye = vec3(0,0,0) - vertex_eye.xyz;

	// 计算漫反射和高光部分
	if (!hasFlag(deferred_material_pass_fromTexture_HasNormalMap))
	{
		for (int i = 0; i < GM_speculars_count; i++)
		{
			vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_speculars[i].lightPosition, 1)).xyz;
			vec3 lightDirection_eye = lightPosition_eye + eyeDirection_eye;
			deferred_light_pass_calcDiffuseAndSpecular(GM_speculars[i], lightDirection_eye, eyeDirection_eye, deferred_light_pass_fromTexture_Normal_eye);
		}
		model3d_calculateRefractionByNormalWorld(deferred_light_pass_fromTexture_Normal.xyz);
	}
	else
	{
		vec3 normal_tangent = deferred_light_pass_fromTexture_NormalMap.rgb * 2.0 - 1.0;
		mat3 TBN = transpose(mat3(
			deferred_light_pass_fromTexture_Tangent_eye,
			deferred_light_pass_fromTexture_Bitangent_eye,
			deferred_light_pass_fromTexture_Normal_eye
		));
		for (int i = 0; i < GM_speculars_count; i++)
		{
			vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_speculars[i].lightPosition, 1)).xyz;
			vec3 lightDirection_eye = lightPosition_eye + eyeDirection_eye;
			vec3 lightDirection_tangent = TBN * lightDirection_eye;
			vec3 eyeDirection_tangent = TBN * eyeDirection_eye;

			deferred_light_pass_calcDiffuseAndSpecular(GM_speculars[i], lightDirection_tangent, eyeDirection_tangent, normal_tangent);
		}
		model3d_calculateRefractionByNormalTangent(TBN, normal_tangent);
	}

	// 计算环境光
	for (int i = 0; i < GM_ambients_count; i++)
	{
		deferred_light_pass_g_ambientLight += deferred_material_pass_fromTexture_Ka * GM_ambients[i].lightColor;
	}
}

void deferred_light_pass_calcColor()
{
	deferred_light_pass_calcLights();

	// 最终结果
	vec3 color = deferred_light_pass_g_ambientLight * deferred_light_pass_fromTexture_TexAmbient
		+ deferred_light_pass_g_diffuseLight * deferred_light_pass_fromTexture_TexDiffuse
		+ deferred_light_pass_g_specularLight
		+ deferred_light_pass_g_refractiveLight;
	_frag_color = vec4(color, 1.0f);
}

void main()
{
	deferred_light_pass_init();
	deferred_light_pass_calcColor();
}
