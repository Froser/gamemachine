#version 330 core

#define MAX_LIGHT_COUNT 10

in vec2 _texCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal_eye;
uniform sampler2D gTexAmbient;
uniform sampler2D gTexDiffuse;
uniform sampler2D gTangent_eye;
uniform sampler2D gBitangent_eye;
uniform sampler2D gNormalMap;
uniform sampler2D gKa;
uniform sampler2D gKd;
uniform sampler2D gKs;
uniform sampler2D gShininess;
uniform sampler2D gHasNormalMap;

// geometries
vec3 tPosition;
vec3 tNormal_eye;
vec3 tTexAmbient;
vec3 tTexDiffuse;
vec3 tTangent_eye;
vec3 tBitangent_eye;
vec3 tNormalMap;

// materials
vec3 tKa;
vec3 tKd;
vec3 tKs;
float tShininess;

// flags, -1 or 1
float tHasNormalMap;

struct GM_light_t
{
	vec3 lightColor;
	vec3 lightPosition;
};
uniform GM_light_t GM_ambients[MAX_LIGHT_COUNT];
uniform GM_light_t GM_speculars[MAX_LIGHT_COUNT];

uniform mat4 GM_view_matrix;

// 调试变量
uniform int GM_debug_draw_normal;

// 相机视角法向量
vec3 g_normal_eye;
// ShadowMap的阴影系数，如果没有ShadowMap则为1
vec3 g_ambientLight;
vec3 g_diffuseLight;
vec3 g_specularLight;

out vec4 frag_color;

bool hasFlag(float flag)
{
	return flag > 0;
}

void init()
{
	g_ambientLight = g_diffuseLight = g_specularLight = vec3(0);
	tPosition = texture(gPosition, _texCoords).rgb;
	tNormal_eye = texture(gNormal_eye, _texCoords).rgb;
	tTexAmbient = texture(gTexAmbient, _texCoords).rgb;
	tTexDiffuse = texture(gTexDiffuse, _texCoords).rgb;
	tTangent_eye = texture(gTangent_eye, _texCoords).rgb;
	tBitangent_eye = texture(gBitangent_eye, _texCoords).rgb;
	tNormalMap = texture(gNormalMap, _texCoords).rgb;
	tKa = texture(gKa, _texCoords).rgb;
	tKd = texture(gKd, _texCoords).rgb;
	tKs = texture(gKs, _texCoords).rgb;
	tShininess = texture(gShininess, _texCoords).r;
	tHasNormalMap = texture(gHasNormalMap, _texCoords).r;
}

void calcDiffuseAndSpecular(GM_light_t light, vec3 lightDirection, vec3 eyeDirection, vec3 normal)
{
	vec3 N = normalize(normal);
	vec3 L = normalize(lightDirection);

	//diffuse:
	{
		float diffuseFactor = dot(L, N);
		diffuseFactor = clamp(diffuseFactor, 0.0f, 1.0f);

		g_diffuseLight += diffuseFactor * tKd * light.lightColor;
	}

	// specular:
	{
		vec3 V = normalize(eyeDirection);
		vec3 R = reflect(-L, N);
		float theta = dot(V, R);
		float specularFactor = pow(theta, tShininess);
		specularFactor = clamp(specularFactor, 0.0f, 1.0f);

		g_specularLight += specularFactor * tKs * light.lightColor;
	}
}

void calcLights()
{
	// 由顶点变换矩阵计算法向量变换矩阵
	vec4 vertex_eye = GM_view_matrix * vec4(tPosition, 1);
	vec3 eyeDirection_eye = vec3(0,0,0) - vertex_eye.xyz;

	// 计算漫反射和高光部分
	if (!hasFlag(tHasNormalMap))
	{
		for (int i = 0; i < MAX_LIGHT_COUNT; i++)
		{
			vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_speculars[i].lightPosition, 1)).xyz;
			vec3 lightDirection_eye = lightPosition_eye + eyeDirection_eye;
			calcDiffuseAndSpecular(GM_speculars[i], lightDirection_eye, eyeDirection_eye, tNormal_eye);
		}
	}
	else
	{
		vec3 normal_tangent = tNormalMap.rgb * 2.0 - 1.0;
		for (int i = 0; i < MAX_LIGHT_COUNT; i++)
		{
			vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_speculars[i].lightPosition, 1)).xyz;
			vec3 lightDirection_eye = lightPosition_eye + eyeDirection_eye;
			mat3 TBN = transpose(mat3(
				tTangent_eye,
				tBitangent_eye,
				tNormal_eye
			));
			vec3 lightDirection_tangent = TBN * lightDirection_eye;
			vec3 eyeDirection_tangent = TBN * eyeDirection_eye;

			calcDiffuseAndSpecular(GM_speculars[i], lightDirection_tangent, eyeDirection_tangent, normal_tangent);
		}
	}

	// 计算环境光
	for (int i = 0; i < MAX_LIGHT_COUNT; i++)
	{
		g_ambientLight += tKa * GM_ambients[i].lightColor;
	}
}

void calcColor()
{
	calcLights();

	// 最终结果
	vec3 color = g_ambientLight * tTexAmbient + g_diffuseLight * tTexDiffuse + g_specularLight;
	frag_color = vec4(color, 1.0f);
}

void main()
{
	init();
	calcColor();
}
