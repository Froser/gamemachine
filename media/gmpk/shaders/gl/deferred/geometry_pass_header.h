// [-1, 1] -> [0, 1]
vec4 normalToTexture(vec3 normal)
{
	return vec4((normal + 1) * .5f, 1);
}
