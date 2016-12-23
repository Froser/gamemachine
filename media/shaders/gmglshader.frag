#version 330 core
#define ZERO_BREAK(n) if (n <= 0.001) return;
#define MORE_THAN_ONE_BREAK(n) if (n >= 1) return;

uniform vec4 light_ambient;
uniform vec4 light_ka;
uniform vec4 light_color;
uniform vec4 light_kd;
uniform vec4 light_ks;

in float diffuse;
in float specular;

out vec4 color;

void
main()
{
	vec3 ambientLight = vec3(light_ambient) * vec3(light_ka);

	vec3 diffuseLight = diffuse * vec3(light_color) * vec3(light_kd);

	vec3 specularLight = specular * vec3(light_color) * vec3(light_ks);

	vec3 rgb = min(ambientLight + diffuseLight + specularLight, vec3(1.0f));

    color = vec4(rgb, 1.0f);
}
