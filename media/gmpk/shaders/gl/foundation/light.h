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
	diffuseFactor = clamp(diffuseFactor, 0.0f, 1.0f);
	return diffuseFactor * light.lightColor;
}

vec3 GMLight_DirectLightSpecular(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N, float shininess)
{
	vec3 R = reflect(-lightDirection_N, normal_N);
	float theta = dot(eyeDirection_N, R);
	float specularFactor = pow(theta, shininess);
	specularFactor = clamp(specularFactor, 0.0f, 1.0f);
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