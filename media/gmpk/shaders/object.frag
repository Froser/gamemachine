#version 330 core

in vec4 position_world;
in vec4 _normal;
in vec2 _uv;
in vec4 _tangent;
in vec4 _bitangent;
in vec2 _lightmapuv;
in vec4 shadowCoord;

// 阴影纹理
uniform sampler2DShadow GM_shadow_texture;
uniform int GM_shadow_texture_switch = 0;

#define MAX_TEXTURE_COUNT 3
#define MAX_LIGHT_COUNT 10
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
uniform GM_Material_t GM_material;

uniform mat4 GM_view_matrix;
uniform mat4 GM_model_matrix;

// 调试变量
uniform int GM_debug_draw_normal;

// 相机视角法向量
vec3 g_normal_eye;
// ShadowMap的阴影系数，如果没有ShadowMap则为1
float g_shadeFactor = 0;
vec3 g_ambientLight;
vec3 g_diffuseLight;
vec3 g_specularLight;

out vec4 frag_color;

void init()
{
	g_ambientLight = g_diffuseLight = g_specularLight = vec3(0);
}

float calcuateShadeFactor(vec4 shadowCoord)
{
	if (GM_shadow_texture_switch == 0)
		return 1;

	float shadeFactor = 0.0;
	shadeFactor += textureProjOffset(GM_shadow_texture, shadowCoord, ivec2(-1, -1));
	shadeFactor += textureProjOffset(GM_shadow_texture, shadowCoord, ivec2(1, -1));
	shadeFactor += textureProjOffset(GM_shadow_texture, shadowCoord, ivec2(-1, 1));
	shadeFactor += textureProjOffset(GM_shadow_texture, shadowCoord, ivec2(1, 1));
	shadeFactor /= 4;

	return shadeFactor;
}

float shadeFactorFactor(float shadeFactor)
{
	return min(shadeFactor + 0.3, 1);
}

void calcDiffuseAndSpecular(GM_light_t light, vec3 lightDirection, vec3 eyeDirection, vec3 normal)
{
	vec3 N = normalize(normal);
	vec3 L = normalize(lightDirection);

	//diffuse:
	{
		float diffuseFactor = dot(L, N);
		diffuseFactor = clamp(diffuseFactor, 0.0f, 1.0f);

		g_diffuseLight += diffuseFactor * GM_material.kd * g_shadeFactor * light.lightColor;
	}

	// specular:
	{
		vec3 V = normalize(eyeDirection);
		vec3 R = reflect(-L, N);
		float theta = dot(V, R);
		float specularFactor = pow(theta, GM_material.shininess);
		specularFactor = clamp(specularFactor, 0.0f, 1.0f);

		g_specularLight += specularFactor * GM_material.ks * g_shadeFactor * light.lightColor;
	}
}

void calcLights()
{
	g_shadeFactor = shadeFactorFactor(calcuateShadeFactor(shadowCoord));

	// 由顶点变换矩阵计算法向量变换矩阵
	mat4 normalModelTransform = transpose(inverse(GM_model_matrix));
	mat4 normalEyeTransform = GM_view_matrix * normalModelTransform;

	vec4 vertex_eye = GM_view_matrix * position_world;
	vec3 eyeDirection_eye = vec3(0,0,0) - vertex_eye.xyz;
	// normal的齐次向量最后一位必须位0，因为法线变换不考虑平移
	g_normal_eye = normalize( (normalEyeTransform * vec4(_normal.xyz, 0)).xyz );

	// 计算漫反射和高光部分
	if (GM_normalmap_textures[0].enabled == 0)
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
		vec3 tangent_eye = normalize((normalEyeTransform * vec4(_tangent.xyz, 0)).xyz);
		vec3 bitangent_eye = normalize((normalEyeTransform * vec4(_bitangent.xyz, 0)).xyz);
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
			vec3 normal_tangent = texture(GM_normalmap_textures[0].texture, _uv).rgb * 2.0 - 1.0;

			calcDiffuseAndSpecular(GM_speculars[i], lightDirection_tangent, eyeDirection_tangent, normal_tangent);
		}
	}

	// 计算环境光
	for (int i = 0; i < MAX_LIGHT_COUNT; i++)
	{
		g_ambientLight += GM_material.ka * GM_ambients[i].lightColor * g_shadeFactor;
	}
}

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

void calcColor()
{
	calcLights();

	if (GM_debug_draw_normal == 1)
	{
		// 画眼睛视角的法向量
		frag_color = vec4((g_normal_eye.xyz + 1.f) / 2.f, 1.f);
		return;
	}
	else if (GM_debug_draw_normal == 2)
	{
		// 画世界视角的法向量
		frag_color = vec4((_normal.xyz + 1.f) / 2.f, 1.f);
		return;
	}

	vec3 diffuseTextureColor = calcTexture(GM_diffuse_textures, _uv, MAX_TEXTURE_COUNT);
	vec3 ambientTextureColor = calcTexture(GM_ambient_textures, _uv, MAX_TEXTURE_COUNT) *
		calcTexture(GM_lightmap_textures, _lightmapuv, 1);

	// 最终结果
	vec3 color = g_ambientLight * ambientTextureColor + g_diffuseLight * diffuseTextureColor + g_specularLight;
	frag_color = vec4(color, 1.0f);
}

void main()
{
	init();
	calcColor();
}
