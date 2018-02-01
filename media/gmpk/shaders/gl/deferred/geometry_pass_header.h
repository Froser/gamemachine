#define PROC_GEOMETRY_PASS		0
#define PROC_MATERIAL_PASS		1

// [-1, 1] -> [0, 1]
vec4 normalToTexture(vec3 normal)
{
	vec3 n = normal;
	n.z = -n.z;
	return vec4((n + 1) * .5f, 1);
}
