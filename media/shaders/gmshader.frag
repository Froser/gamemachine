#version 400 core
#define ZERO_BREAK(n) if (n <= 0.001) return;
#define MORE_THAN_ONE_BREAK(n) if (n >= 1) return;

// 阴影纹理
uniform sampler2DShadow shadow_texture;
uniform int has_shadow_texture = 0;

// 环境光纹理
uniform sampler2D ambient_texture;
uniform int has_ambient_texture = 0;

// 环境立方体纹理（绘制天空时）
uniform samplerCube cubemap_texture;
uniform int has_cubemap_texture = 0;

// 环境立方体纹理（反射天空时）
uniform samplerCube reflection_cubemap_texture;
uniform int has_reflection_cubemap_texture = 0;

uniform vec4 light_ambient;
uniform vec4 light_ka;
uniform vec4 light_color;
uniform vec4 light_kd;
uniform vec4 light_ks;
uniform vec4 light_ke;

in float diffuse;
in float specular;
in vec4 shadow_coord;
in vec2 ambient_texture_uv;
in vec3 cubemap_texture_uv;
in vec3 normalized_view_coord;
in vec3 view_vector;
in vec3 view_direction;

out vec4 frag_color;

float calcuateShadeFactor()
{
	if (has_shadow_texture == 0)
		return 0;

	float shadeFactor = 0.0;

	shadeFactor += textureProjOffset(shadow_texture, shadow_coord, ivec2(-1, -1));

	shadeFactor += textureProjOffset(shadow_texture, shadow_coord, ivec2(1, -1));

	shadeFactor += textureProjOffset(shadow_texture, shadow_coord, ivec2(-1, 1));

	shadeFactor += textureProjOffset(shadow_texture, shadow_coord, ivec2(1, 1));

	shadeFactor /= 4;

	return shadeFactor;
}

void drawSky()
{
	vec3 cubemapTextureColor = vec3(texture(cubemap_texture, cubemap_texture_uv));

	frag_color = light_ambient + vec4(cubemapTextureColor, 1.0f);
}

void drawObject()
{
	vec3 ambientTextureColor = has_ambient_texture == 1 ? vec3(texture(ambient_texture, ambient_texture_uv)) : vec3(0);

	vec3 ambientLight = (vec3(light_ambient) + ambientTextureColor) * vec3(light_ka);

	vec3 diffuseLight = diffuse * vec3(light_color) * vec3(light_kd);

	vec3 specularLight = specular * vec3(light_color) * vec3(light_ks);

	float shadeFactor = calcuateShadeFactor();

	// 根据环境反射度来反射天空盒（如果有的话）
	if (has_reflection_cubemap_texture == 1)
	{
		vec3 reflection_coord = reflect(-view_direction, normalized_view_coord) - view_vector;

		// 乘以shadeFactor是因为阴影会遮挡反射光
		if (light_ke.x > 0 && light_ke.y > 0 && light_ke.z > 0)
		{
			vec3 color_from_reflection = shadeFactor * (texture(reflection_cubemap_texture, reflection_coord) * vec3(light_ke));

			ambientLight += color_from_reflection;
		}
	}

	vec3 rgb = ambientLight + min(shadeFactor + 0.3, 1) * (diffuseLight + specularLight);

	frag_color = vec4(rgb, 1.0f);
}

void main()
{
	if (has_cubemap_texture == 1)
	{
		// 如果存在立方体纹理，说明是环境（天空）的绘制，那么只考虑环境光，并将Ka全部设置为1
		drawSky();
	}
	else
	{
		// 按照正常的流程绘制光照、阴影
		drawObject();
	}
}
