#define PROC_GEOMETRY_PASS		0
#define PROC_MATERIAL_PASS		1

// [-1, 1] -> [0, 1]
vec3 normalToTexture(vec3 normal)
{
	return (normal + 1) * .5f;
}
