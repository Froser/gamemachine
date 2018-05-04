// 光照相关
#define MAX_LIGHT_COUNT 50
struct GM_light_t
{
	vec3 lightColor;
	vec3 lightPosition;
	int lightType; //0表示环境光，1表示直接光
};

uniform GM_light_t GM_lights[MAX_LIGHT_COUNT];
uniform int GM_lightCount = 0;

// 光源种类
const int GM_AmbientLight = 0;
const int GM_DirectLight = 1;

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
	return light.lightColor;
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
	return diffuseFactor * light.lightColor;
}

vec3 GMLight_DirectLightSpecular(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N, float shininess)
{
	vec3 R = reflect(-lightDirection_N, normal_N);
	float theta = dot(eyeDirection_N, R);
	float specularFactor = pow(theta, shininess);
	specularFactor = saturate(specularFactor);
	return specularFactor * light.lightColor;
}

// 代理方法
vec3 GMLight_Ambient(GM_light_t light)
{
	if (light.lightType == GM_AmbientLight)
		return GMLight_AmbientLightAmbient(light);
	return GMLight_DirectLightAmbient(light);
}

vec3 GMLight_Diffuse(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N)
{
	if (light.lightType == GM_AmbientLight)
		return GMLight_AmbientLightDiffuse(light, lightDirection_N, eyeDirection_N, normal_N);
	return GMLight_DirectLightDiffuse(light, lightDirection_N, eyeDirection_N, normal_N);
}

vec3 GMLight_Specular(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N, float shininess)
{
	if (light.lightType == GM_AmbientLight)
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
    vec3 Ks;
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

vec4 PS_3D_CalculateColor(PS_3D_INPUT vertex)
{
	vec3 ambientLight = vec3(0, 0, 0);
	vec3 diffuseLight = vec3(0, 0, 0);
	vec3 specularLight = vec3(0, 0, 0);
	vec3 refractionLight = vec3(0, 0, 0);
	vec3 eyeDirection_eye = -(GM_view_matrix * vec4(vertex.WorldPos, 1)).xyz;
	vec3 eyeDirection_eye_N = normalize(eyeDirection_eye) ;

	// 计算漫反射和高光部分
	if (!vertex.HasNormalMap)
	{
		for (int i = 0; i < GM_lightCount; i++)
		{
			vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_lights[i].lightPosition, 1)).xyz;
			vec3 lightDirection_eye_N = normalize(lightPosition_eye + eyeDirection_eye);
			ambientLight += GMLight_Ambient(GM_lights[i]);
			diffuseLight += GMLight_Diffuse(GM_lights[i], lightDirection_eye_N, eyeDirection_eye_N, vertex.Normal_Eye_N);
			specularLight += GMLight_Specular(GM_lights[i], lightDirection_eye_N, eyeDirection_eye_N, vertex.Normal_Eye_N, vertex.Shininess);
			if (GM_lights[i].lightType == GM_AmbientLight)
				refractionLight += calculateRefractionByNormalWorld(vertex.WorldPos, vertex.Normal_World_N, vertex.Refractivity);
		}
	}
	else
	{
		for (int i = 0; i < GM_lightCount; i++)
		{
			vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_lights[i].lightPosition, 1)).xyz;
			vec3 lightDirection_eye_N = normalize(lightPosition_eye + eyeDirection_eye);
			vec3 lightDirection_tangent_N = normalize(vertex.TangentSpace.TBN * lightDirection_eye_N);
			vec3 eyeDirection_tangent_N = normalize(vertex.TangentSpace.TBN * eyeDirection_eye_N);

			ambientLight += GMLight_Ambient(GM_lights[i]);
			diffuseLight += GMLight_Diffuse(GM_lights[i], lightDirection_tangent_N, eyeDirection_tangent_N, vertex.TangentSpace.Normal_Tangent_N);
			specularLight += GMLight_Specular(GM_lights[i], lightDirection_tangent_N, eyeDirection_tangent_N, vertex.TangentSpace.Normal_Tangent_N, vertex.Shininess);
			if (GM_lights[i].lightType == GM_AmbientLight)
				refractionLight += calculateRefractionByNormalTangent(vertex.WorldPos, vertex.TangentSpace, vertex.Refractivity);
		}
	}
	vec3 finalColor =	saturate(vertex.AmbientLightmapTexture) * ambientLight +
						saturate(vertex.DiffuseTexture) * diffuseLight +
						specularLight * vertex.Ks +
						refractionLight;
	return vec4(finalColor, 1);
}