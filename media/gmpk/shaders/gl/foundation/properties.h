// 位置
uniform vec4 GM_view_position;

// 阴影纹理
uniform sampler2DShadow GM_shadow_texture;
uniform int GM_shadow_texture_switch = 0;

struct GM_texture_t
{
	sampler2D texture;
	float scroll_s;
	float scroll_t;
	float scale_s;
	float scale_t;
	int enabled;
};
uniform GM_texture_t GM_ambient_texture;
uniform GM_texture_t GM_diffuse_texture;
uniform GM_texture_t GM_specular_texture;
uniform GM_texture_t GM_lightmap_texture;
uniform GM_texture_t GM_normalmap_texture;
uniform GM_texture_t GM_albedo_texture;
uniform GM_texture_t GM_metallic_metallic_roughness_texture;
uniform samplerCube GM_cubemap_texture; //CubeMap作为单独一个纹理

vec4 sampleTextures(GM_texture_t tex, vec2 uv)
{
	return tex.enabled == 1
			? vec4(texture(tex.texture, uv * vec2(tex.scale_s, tex.scale_t) + vec2(tex.scroll_s, tex.scroll_t)))
			: vec4(0);
}

struct GM_Material_t
{
	vec3 ka;
	vec3 kd;
	vec3 ks;
	float shininess;
	float refractivity;
};
uniform GM_Material_t GM_material;
