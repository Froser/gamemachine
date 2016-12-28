#version 400 core
#define ZERO_BREAK(n) if (n <= 0.001) return;
#define MORE_THAN_ONE_BREAK(n) if (n >= 1) return;

uniform sampler2DShadow depth_texture;
uniform sampler2D ambient_texture;
uniform vec4 light_ambient;
uniform vec4 light_ka;
uniform vec4 light_color;
uniform vec4 light_kd;
uniform vec4 light_ks;

in float diffuse;
in float specular;
in vec4 shadow_coord;
in vec2 ambient_texture_uv;

out vec4 frag_color;

float calcuateShadeFactor()
{
	float shadeFactor = 0.0;
	shadeFactor += textureProjOffset(depth_texture, shadow_coord, ivec2(-1, -1));

	shadeFactor += textureProjOffset(depth_texture, shadow_coord, ivec2(1, -1));

	shadeFactor += textureProjOffset(depth_texture, shadow_coord, ivec2(-1, 1));

	shadeFactor += textureProjOffset(depth_texture, shadow_coord, ivec2(1, 1));

	shadeFactor /= 4;

	return shadeFactor;
}

void main()
{
	vec3 ambientTextureColor = vec3(texture(ambient_texture, ambient_texture_uv));

	vec3 ambientLight = vec3(light_ambient) * (vec3(light_ka) + ambientTextureColor);

	vec3 diffuseLight = diffuse * vec3(light_color) * vec3(light_kd);

	vec3 specularLight = specular * vec3(light_color) * vec3(light_ks);

	float shadeFactor = calcuateShadeFactor();

	vec3 rgb = ambientLight + min(shadeFactor + 0.3, 1) * (diffuseLight + specularLight);

	frag_color = vec4(rgb, 1.0f);
}
