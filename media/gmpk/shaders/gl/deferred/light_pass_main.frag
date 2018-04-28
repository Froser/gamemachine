#version 410
#include "../foundation/foundation.h"
#include "../foundation/properties.h"
#include "../foundation/light.h"

in vec2 _uv;
out vec4 _frag_color;

uniform sampler2D deferred_light_pass_gPosition_Refractivity;
uniform sampler2D deferred_light_pass_gNormal;
uniform sampler2D deferred_light_pass_gTexAmbient;
uniform sampler2D deferred_light_pass_gTexDiffuse;
uniform sampler2D deferred_light_pass_gTangent_eye;
uniform sampler2D deferred_light_pass_gBitangent_eye;
uniform sampler2D deferred_light_pass_gNormalMap_bNormalMap;
uniform sampler2D deferred_light_pass_gKs_Shininess;

// geometries
vec3 deferred_light_pass_fromTexture_Position;
vec3 deferred_light_pass_fromTexture_Normal;
vec3 deferred_light_pass_fromTexture_Normal_eye;
vec3 deferred_light_pass_fromTexture_TexAmbient;
vec3 deferred_light_pass_fromTexture_TexDiffuse;
vec3 deferred_light_pass_fromTexture_Tangent_eye;
vec3 deferred_light_pass_fromTexture_Bitangent_eye;
vec3 deferred_light_pass_fromTexture_NormalMap;
vec3 deferred_light_pass_fromTexture_Ks;
float deferred_light_pass_fromTexture_Shininess;
float deferred_light_pass_fromTexture_HasNormalMap = 0;
float deferred_light_pass_fromTexture_Refractivity = 0;

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

	vec4 positionShininess = texture(deferred_light_pass_gPosition_Refractivity, _uv);
	deferred_light_pass_fromTexture_Position = positionShininess.rgb;
	deferred_light_pass_fromTexture_Refractivity = positionShininess.a;

	deferred_light_pass_fromTexture_Normal = textureToNormal(texture(deferred_light_pass_gNormal, _uv).rgb);
	deferred_light_pass_fromTexture_Normal_eye = (GM_view_matrix * vec4(deferred_light_pass_fromTexture_Normal, 0)).rgb;
	deferred_light_pass_fromTexture_TexAmbient = texture(deferred_light_pass_gTexAmbient, _uv).rgb;
	deferred_light_pass_fromTexture_TexDiffuse = texture(deferred_light_pass_gTexDiffuse, _uv).rgb;
	deferred_light_pass_fromTexture_Tangent_eye = textureToNormal(texture(deferred_light_pass_gTangent_eye, _uv).rgb);
	deferred_light_pass_fromTexture_Bitangent_eye = textureToNormal(texture(deferred_light_pass_gBitangent_eye, _uv).rgb);

	vec4 normalMapHasNormalMap = texture(deferred_light_pass_gNormalMap_bNormalMap, _uv);
	deferred_light_pass_fromTexture_NormalMap = normalMapHasNormalMap.rgb;
	deferred_light_pass_fromTexture_HasNormalMap = normalMapHasNormalMap.a;

	vec4 ksShininess = texture(deferred_light_pass_gKs_Shininess, _uv);
	deferred_light_pass_fromTexture_Ks = ksShininess.rgb;
	deferred_light_pass_fromTexture_Shininess = ksShininess.a;
}

void deferred_light_pass_calcDiffuseAndSpecular(GM_light_t light, vec3 lightDirection, vec3 eyeDirection, vec3 normal)
{
	vec3 N = normalize(normal);
	vec3 L = normalize(lightDirection);

	//diffuse:
	{
		float diffuseFactor = dot(L, N);
		diffuseFactor = clamp(diffuseFactor, 0.0f, 1.0f);

		deferred_light_pass_g_diffuseLight += diffuseFactor * light.lightColor;
	}

	// specular:
	{
		vec3 V = normalize(eyeDirection);
		vec3 R = reflect(-L, N);
		float theta = dot(V, R);
		float specularFactor = pow(theta, deferred_light_pass_fromTexture_Shininess);
		specularFactor = clamp(specularFactor, 0.0f, 1.0f);

		deferred_light_pass_g_specularLight += specularFactor * deferred_light_pass_fromTexture_Ks * light.lightColor;
	}
}

void model3d_calculateRefractionByNormalWorld(vec3 normal_world)
{
	if (deferred_light_pass_fromTexture_Refractivity == 0.f)
		return;

	vec3 I = normalize(deferred_light_pass_fromTexture_Position - GM_view_position.rgb);
	vec3 R = refract(I, normal_world, deferred_light_pass_fromTexture_Refractivity);
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
	if (!hasFlag(deferred_light_pass_fromTexture_HasNormalMap))
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
		deferred_light_pass_g_ambientLight += GM_ambients[i].lightColor;
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
	if (color == vec3(0, 0, 0))
		discard;
	_frag_color = vec4(color, 1.0f);
}

void main()
{
	deferred_light_pass_init();
	deferred_light_pass_calcColor();
}
