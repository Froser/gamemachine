// 位置
uniform vec4 GM_view_position;

// 调试变量
uniform int GM_debug_draw_normal;

// 阴影纹理
uniform sampler2DShadow GM_shadow_texture;
uniform int GM_shadow_texture_switch = 0;

#define MAX_TEXTURE_COUNT 3
#define MAX_LIGHT_COUNT 5
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
uniform samplerCube GM_cubemap_texture; //CubeMap作为单独一个纹理

// 光照
struct GM_light_t
{
	vec3 lightColor;
	vec3 lightPosition;
};
uniform GM_light_t GM_lights[MAX_LIGHT_COUNT];

uniform GM_light_t GM_ambients[MAX_LIGHT_COUNT]; //-
uniform GM_light_t GM_speculars[MAX_LIGHT_COUNT]; //-
uniform int GM_ambients_count; //-
uniform int GM_speculars_count; //-
subroutine vec3 GM_LightAmbient(GM_light_t light);
subroutine vec3 GM_LightDiffuse(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N);
subroutine vec3 GM_LightSpecular(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N);
subroutine uniform GM_LightAmbient GM_lightAmbient[MAX_LIGHT_COUNT];
subroutine uniform GM_LightDiffuse GM_lightDiffuse[MAX_LIGHT_COUNT];
subroutine uniform GM_LightSpecular GM_lightSpecular[MAX_LIGHT_COUNT];
uniform int GM_lightCount = 0;

struct GM_Material_t
{
	vec3 ka;
	vec3 kd;
	vec3 ks;
	float shininess;
	float refractivity;
};
uniform GM_Material_t GM_material;
