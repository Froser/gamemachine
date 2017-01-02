#version 330 core

// 阴影纹理
uniform sampler2DShadow GM_shadow_texture;
uniform int GM_shadow_texture_switch = 0;

// 环境光纹理
uniform sampler2D GM_ambient_texture;
uniform int GM_ambient_texture_switch = 0;

// 环境立方体纹理（绘制天空时）
uniform samplerCube GM_cubemap_texture;
uniform int GM_cubemap_texture_switch = 0;

// 环境立方体纹理（反射天空时）
uniform samplerCube GM_reflection_cubemap_texture;
uniform int GM_reflection_cubemap_texture_switch = 0;

uniform vec4 GM_light_ambient;
uniform vec4 GM_light_ka;
uniform vec4 GM_light_color;
uniform vec4 GM_light_kd;
uniform vec4 GM_light_ks;
uniform vec4 GM_light_ke;

struct _Coords
{
    // 顶点世界坐标
    vec4 worldCoord;

    // 顶点在视角变换后的坐标
    vec4 modelViewCoord;

    // 投影后的最终坐标
    vec4 position;

    // 变换后的世界坐标标准法向量
    vec3 worldNormalCoord;

    // 灯光照射方向（目前灯光最多数量为1）
    vec3 lightDirection;

    // 视角方向
    vec3 viewDirection;

    // 阴影坐标
    vec4 shadowCoord;
};

struct _TextureUVs
{
    // 环境光贴图坐标
    vec2 ambientUV;

    // CubeMap贴图坐标
    vec3 cubemapUV;
};

struct _LightFactors
{
    // 漫反射系数
    float diffuse;

    // 镜面反射系数
    float specular;
};

in _Coords coords;
in _TextureUVs textureUVs;
in _LightFactors lightFactors;

out vec4 frag_color;

float calcuateShadeFactor(_Coords coords)
{
	if (GM_shadow_texture_switch == 0)
		return 0;

	float shadeFactor = 0.0;
	shadeFactor += textureProjOffset(GM_shadow_texture, coords.shadowCoord, ivec2(-1, -1));
	shadeFactor += textureProjOffset(GM_shadow_texture, coords.shadowCoord, ivec2(1, -1));
	shadeFactor += textureProjOffset(GM_shadow_texture, coords.shadowCoord, ivec2(-1, 1));
	shadeFactor += textureProjOffset(GM_shadow_texture, coords.shadowCoord, ivec2(1, 1));
	shadeFactor /= 4;

	return shadeFactor;
}

void drawSky(_TextureUVs uvs)
{
	vec3 cubemapTextureColor = vec3(texture(GM_cubemap_texture, uvs.cubemapUV));
	frag_color = GM_light_ambient + vec4(cubemapTextureColor, 1.0f);
}

void drawObject()
{
	// 计算环境光和环境光贴图
	vec3 ambientTextureColor = GM_ambient_texture_switch == 1 ? vec3(texture(GM_ambient_texture, textureUVs.ambientUV)) : vec3(0);
	vec3 ambientLight = (vec3(GM_light_ambient) + ambientTextureColor) * vec3(GM_light_ka);

	// 计算点光源
	vec3 diffuseLight = lightFactors.diffuse * vec3(GM_light_color) * vec3(GM_light_kd);
	vec3 specularLight = lightFactors.specular * vec3(GM_light_color) * vec3(GM_light_ks);

	// 计算阴影系数
	float shadeFactor = calcuateShadeFactor(coords);

	// 根据环境反射度来反射天空盒（如果有的话）
	if (GM_reflection_cubemap_texture_switch == 1)
	{
		vec3 reflectionCoord = reflect(-coords.viewDirection, coords.worldNormalCoord.xyz);
		// 乘以shadeFactor是因为阴影会遮挡反射光
		if (GM_light_ke.x > 0 && GM_light_ke.y > 0 && GM_light_ke.z > 0)
		{
			vec3 color_from_reflection = shadeFactor * (texture(GM_reflection_cubemap_texture, reflectionCoord).rgb * vec3(GM_light_ke));
			ambientLight += color_from_reflection;
		}
	}

	// 最终结果
	vec3 color = ambientLight + min(shadeFactor + 0.3, 1) * (diffuseLight + specularLight);
	frag_color = vec4(color, 1.0f);
}

void main()
{
	if (GM_cubemap_texture_switch == 1)
	{
		// 如果存在立方体纹理，说明是环境（天空）的绘制，那么只考虑环境光，并将Ka全部设置为1
		drawSky(textureUVs);
	}
	else
	{
		// 按照正常的流程绘制光照、阴影
		drawObject();
	}
}
