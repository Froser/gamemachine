#version 330 core

struct GM_Material_t
{
	vec3 ka;
	vec3 kd;
	vec3 ks;
	float shininess;
};
uniform GM_Material_t GM_material;

layout (location = 0) out vec3 gKa;
layout (location = 1) out vec3 gKd;
layout (location = 2) out vec3 gKs;

void main()
{
	gKa = GM_material.ka;
	gKd = GM_material.kd;
	gKs = GM_material.ks;
}
