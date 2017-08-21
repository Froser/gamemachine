#version 330 core

#define MAX_LIGHT_COUNT 10

in vec4 _position_world;
in vec2 _texCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gTexAmbient;
uniform sampler2D gTexDiffuse;
uniform sampler2D gTangent;
uniform sampler2D gBitangent;
uniform sampler2D gNormalMap;

vec3 tPosition;
vec3 tNormal;
vec3 tTexAmbient;
vec3 tTexDiffuse;
vec3 tTangent;
vec3 tBitangent;
vec3 tNormalMap;

struct GM_light_t
{
	vec3 lightColor;
	vec3 lightPosition;
};
uniform GM_light_t GM_ambients[MAX_LIGHT_COUNT];
uniform GM_light_t GM_speculars[MAX_LIGHT_COUNT];

struct GM_Material_t
{
	vec3 ka;
	vec3 kd;
	vec3 ks;
	float shininess;
};
GM_Material_t GM_material;

uniform mat4 GM_view_matrix;
uniform mat4 GM_model_matrix;

// 调试变量
uniform int GM_debug_draw_normal;

// 相机视角法向量
vec3 g_normal_eye;
// ShadowMap的阴影系数，如果没有ShadowMap则为1
vec3 g_ambientLight;
vec3 g_diffuseLight;
vec3 g_specularLight;

out vec4 frag_color;

void init()
{
	tPosition = texture(gPosition, _texCoords).rgb;
	tNormal = texture(gNormal, _texCoords).rgb;
	tTexAmbient = texture(gTexAmbient, _texCoords).rgb;
	tTexDiffuse = texture(gTexDiffuse, _texCoords).rgb;
	tTangent = texture(gTangent, _texCoords).rgb;
	tBitangent = texture(gBitangent, _texCoords).rgb;
	tNormalMap = texture(gNormalMap, _texCoords).rgb;
	g_ambientLight = g_diffuseLight = g_specularLight = vec3(0);

}

void calcDiffuseAndSpecular(GM_light_t light, vec3 lightDirection, vec3 eyeDirection, vec3 normal)
{
	vec3 N = normalize(normal);
	vec3 L = normalize(lightDirection);

	//diffuse:
	{
		float diffuseFactor = dot(L, N);
		diffuseFactor = clamp(diffuseFactor, 0.0f, 1.0f);

		g_diffuseLight += diffuseFactor * GM_material.kd * light.lightColor;
	}

	// specular:
	{
		vec3 V = normalize(eyeDirection);
		vec3 R = reflect(-L, N);
		float theta = dot(V, R);
		float specularFactor = pow(theta, GM_material.shininess);
		specularFactor = clamp(specularFactor, 0.0f, 1.0f);

		g_specularLight += specularFactor * GM_material.ks * light.lightColor;
	}
}

void calcLights()
{
	// 由顶点变换矩阵计算法向量变换矩阵
	mat4 normalModelTransform = transpose(inverse(GM_model_matrix));
	mat4 normalEyeTransform = GM_view_matrix * normalModelTransform;

	vec4 vertex_eye = GM_view_matrix * _position_world;
	vec3 eyeDirection_eye = vec3(0,0,0) - vertex_eye.xyz;
	// normal的齐次向量最后一位必须位0，因为法线变换不考虑平移
	g_normal_eye = normalize( (normalEyeTransform * vec4(tNormal, 0)).xyz );

	// 计算漫反射和高光部分
	if (true) //TODO
	{
		for (int i = 0; i < MAX_LIGHT_COUNT; i++)
		{
			vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_speculars[i].lightPosition, 1)).xyz;
			vec3 lightDirection_eye = lightPosition_eye + eyeDirection_eye;
			calcDiffuseAndSpecular(GM_speculars[i], lightDirection_eye, eyeDirection_eye, g_normal_eye);
		}
	}
	else
	{
		vec3 normal_tangent = tNormalMap.rgb * 2.0 - 1.0;
		vec3 tangent_eye = normalize((normalEyeTransform * vec4(tTangent.xyz, 0)).xyz);
		vec3 bitangent_eye = normalize((normalEyeTransform * vec4(tBitangent.xyz, 0)).xyz);
		for (int i = 0; i < MAX_LIGHT_COUNT; i++)
		{
			vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_speculars[i].lightPosition, 1)).xyz;
			vec3 lightDirection_eye = lightPosition_eye + eyeDirection_eye;
			mat3 TBN = transpose(mat3(
				tangent_eye,
				bitangent_eye,
				g_normal_eye.xyz
			));
			vec3 lightDirection_tangent = TBN * lightDirection_eye;
			vec3 eyeDirection_tangent = TBN * eyeDirection_eye;

			calcDiffuseAndSpecular(GM_speculars[i], lightDirection_tangent, eyeDirection_tangent, normal_tangent);
		}
	}

	// 计算环境光
	for (int i = 0; i < MAX_LIGHT_COUNT; i++)
	{
		g_ambientLight += GM_material.ka * GM_ambients[i].lightColor;
	}
}

void calcColor()
{
	GM_material.kd = vec3(1,1,1);
	calcLights();

	// 最终结果
	vec3 color = g_ambientLight * tTexAmbient + g_diffuseLight * tTexDiffuse + g_specularLight;
	frag_color = vec4(g_specularLight.rgb, 1.0f);
}

void main()
{
	init();
	calcColor();
}
