uniform sampler2D gFramebuffer;

// Effect switches
uniform int GM_effects_inversion;

bool hasEffect(int effect)
{
	return GM_effects_inversion == 1;
}

vec3 inv(sampler2D t, vec2 uv)
{
    vec3 color = texture(t, uv).rgb;
    return vec3(1.f - color.r, 1.f - color.g, 1.f - color.b);
}

vec3 effects(sampler2D t, vec2 uv)
{
	vec3 result;
	if (hasEffect(GM_effects_inversion))
    	result += inv(t, uv);

    return result;
}
