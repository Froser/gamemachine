// 光照相关
#define MAX_LIGHT_COUNT 50
struct GM_light_t
{
	vec3 LightColor;
	vec3 LightPosition;
	int LightType; //0表示环境光，1表示直接光
};

uniform GM_light_t GM_lights[MAX_LIGHT_COUNT];
uniform int GM_lightCount = 0;

// 光源种类
const int GM_AmbientLight = 0;
const int GM_DirectLight = 1;

// Gamma校正
uniform int GM_gammaCorrection;
uniform float GM_gamma;
vec3 calculateGammaCorrection(vec3 factor)
{
    if (GM_gammaCorrection == 0)
        return factor;

    float inverseGamma = 1.0f / GM_gamma;
    return pow(factor, vec3(inverseGamma, inverseGamma, inverseGamma));
}

// 光照实现
vec3 saturate(vec3 vector)
{
	return clamp(vector, 0.0f, 1.0f);
}

vec4 saturate(vec4 vector)
{
	return clamp(vector, 0.0f, 1.0f);
}

float saturate(float f)
{
	return clamp(f, 0.0f, 1.0f);
}

vec3 GMLight_AmbientLightAmbient(GM_light_t light)
{
	return light.LightColor;
}

vec3 GMLight_AmbientLightDiffuse(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N)
{
	return vec3(0, 0, 0);
}

vec3 GMLight_AmbientLightSpecular(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N, float shininess)
{
	return vec3(0, 0, 0);
}

vec3 GMLight_DirectLightAmbient(GM_light_t light)
{
	return vec3(0, 0, 0);
}

vec3 GMLight_DirectLightDiffuse(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N)
{
	float diffuseFactor = dot(lightDirection_N, normal_N);
	diffuseFactor = saturate(diffuseFactor);
	return diffuseFactor * light.LightColor;
}

vec3 GMLight_DirectLightSpecular(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N, float shininess)
{
	vec3 R = normalize(reflect(-lightDirection_N, normal_N));
	float theta = max(dot(eyeDirection_N, R), 0);
	float specularFactor = (theta == 0 && shininess == 0) ? 0 : pow(theta, shininess);
	specularFactor = saturate(specularFactor);
	return specularFactor * light.LightColor;
}

// 代理方法
vec3 GMLight_Ambient(GM_light_t light)
{
	if (light.LightType == GM_AmbientLight)
		return GMLight_AmbientLightAmbient(light);
	return GMLight_DirectLightAmbient(light);
}

vec3 GMLight_Diffuse(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N)
{
	if (light.LightType == GM_AmbientLight)
		return GMLight_AmbientLightDiffuse(light, lightDirection_N, eyeDirection_N, normal_N);
	return GMLight_DirectLightDiffuse(light, lightDirection_N, eyeDirection_N, normal_N);
}

vec3 GMLight_Specular(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N, float shininess)
{
	if (light.LightType == GM_AmbientLight)
		return GMLight_AmbientLightSpecular(light, lightDirection_N, eyeDirection_N, normal_N, shininess);
	return GMLight_DirectLightSpecular(light, lightDirection_N, eyeDirection_N, normal_N, shininess);
}


/////////////////////////////////////////////////////////////////////
// 基本光照流程
struct TangentSpace
{
	vec3 Normal_Tangent_N;
	mat3 TBN;
};

struct PS_3D_INPUT
{
	vec3 WorldPos;            // 世界坐标
	vec3 Normal_World_N;      // 世界法线
	vec3 Normal_Eye_N;        // 眼睛空间法向量
	TangentSpace TangentSpace;  // 切线空间
	bool HasNormalMap;          // 是否有法线贴图
	vec3 AmbientLightmapTexture;
	vec3 DiffuseTexture;
	vec3 SpecularTexture;
	float Shininess;
	float Refractivity; 
};

vec3 calculateRefractionByNormalWorld(vec3 worldPos, vec3 normal_world_N, float refractivity)
{
	if (refractivity == 0.f)
		return vec3(0, 0, 0);

	vec3 I = normalize(worldPos - GM_view_position.xyz);
	vec3 R = refract(I, normal_world_N, refractivity);
	return texture(GM_cubemap_texture, vec3(R.x, R.y, R.z)).rgb;
}

vec3 calculateRefractionByNormalTangent(vec3 worldPos, TangentSpace tangentSpace, float refractivity)
{
	if (refractivity == 0.f)
		return vec3(0, 0, 0);
	
	// 如果是切线空间，计算会复杂点，要将切线空间的法线换算回世界空间
	vec3 normal_world_N = normalize(mat3(GM_inverse_view_matrix) * transpose(tangentSpace.TBN) * tangentSpace.Normal_Tangent_N);
	return calculateRefractionByNormalWorld(worldPos, normal_world_N, refractivity);
}

float calculateShadow(mat4 shadowMatrix, vec4 worldPos, vec3 normal_N)
{
	if (GM_shadowInfo.HasShadow == 0)
		return 1.0f;

	vec4 fragPos = shadowMatrix * worldPos;
	vec3 projCoords = fragPos.xyz / fragPos.w;
	if (projCoords.z > 1.0f)
		return 1.0f;
	projCoords = projCoords * 0.5f + 0.5f;

	float bias = (GM_shadowInfo.BiasMin == GM_shadowInfo.BiasMax) ? GM_shadowInfo.BiasMin : max(GM_shadowInfo.BiasMax * (1.0 - dot(normal_N, normalize(worldPos.xyz - GM_shadowInfo.Position.xyz))), GM_shadowInfo.BiasMin);
	float closestDepth = texture(GM_shadowInfo.ShadowMap, projCoords.xy).r;
	return projCoords.z - bias > closestDepth ? 0.f : 1.f;
}

vec4 PS_3D_CalculateColor(PS_3D_INPUT vertex)
{
	float factor_Shadow = calculateShadow(GM_shadowInfo.ShadowMatrix, vec4(vertex.WorldPos, 1), vertex.Normal_World_N);
	vec3 ambientLight = vec3(0, 0, 0);
	vec3 diffuseLight = vec3(0, 0, 0);
	vec3 specularLight = vec3(0, 0, 0);
	vec3 refractionLight = vec3(0, 0, 0);
	vec3 eyeDirection_eye = -(GM_view_matrix * vec4(vertex.WorldPos, 1)).xyz;
	vec3 eyeDirection_eye_N = normalize(eyeDirection_eye);

	// 计算漫反射和高光部分
	if (!vertex.HasNormalMap)
	{
		for (int i = 0; i < GM_lightCount; i++)
		{
			vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_lights[i].LightPosition, 1)).xyz;
			vec3 lightDirection_eye_N = normalize(lightPosition_eye + eyeDirection_eye);
			ambientLight += GMLight_Ambient(GM_lights[i]);
			diffuseLight += GMLight_Diffuse(GM_lights[i], lightDirection_eye_N, eyeDirection_eye_N, vertex.Normal_Eye_N);
			specularLight += GMLight_Specular(GM_lights[i], lightDirection_eye_N, eyeDirection_eye_N, vertex.Normal_Eye_N, vertex.Shininess);
			if (GM_lights[i].LightType == GM_AmbientLight)
				refractionLight += calculateRefractionByNormalWorld(vertex.WorldPos, vertex.Normal_World_N, vertex.Refractivity);
		}
	}
	else
	{
		for (int i = 0; i < GM_lightCount; i++)
		{
			vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_lights[i].LightPosition, 1)).xyz;
			vec3 lightDirection_eye_N = normalize(lightPosition_eye + eyeDirection_eye);
			vec3 lightDirection_tangent_N = normalize(vertex.TangentSpace.TBN * lightDirection_eye_N);
			vec3 eyeDirection_tangent_N = normalize(vertex.TangentSpace.TBN * eyeDirection_eye_N);

			ambientLight += GMLight_Ambient(GM_lights[i]);
			diffuseLight += GMLight_Diffuse(GM_lights[i], lightDirection_tangent_N, eyeDirection_tangent_N, vertex.TangentSpace.Normal_Tangent_N);
			specularLight += GMLight_Specular(GM_lights[i], lightDirection_tangent_N, eyeDirection_tangent_N, vertex.TangentSpace.Normal_Tangent_N, vertex.Shininess);
			if (GM_lights[i].LightType == GM_AmbientLight)
				refractionLight += calculateRefractionByNormalTangent(vertex.WorldPos, vertex.TangentSpace, vertex.Refractivity);
		}
	}
	vec3 finalColor =	saturate(vertex.AmbientLightmapTexture) * calculateGammaCorrection(ambientLight) +
						saturate(vertex.DiffuseTexture) * calculateGammaCorrection(diffuseLight) * factor_Shadow +
						specularLight * calculateGammaCorrection(vertex.SpecularTexture) * factor_Shadow +
						refractionLight;
	return vec4(finalColor, 1);
}