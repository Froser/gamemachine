uniform sampler2D GM_framebuffer;

uniform float GM_effects_texture_offset_x;
uniform float GM_effects_texture_offset_y;

// Effect switches
uniform int GM_filter = 0;

const int Filter_None = 0;
const int Filter_Inversion = 1;
const int Filter_Sharpen = 2;
const int Filter_Blur = 3;
const int Filter_Grayscale = 4;
const int Filter_EdgeDetect = 5;

vec3 kernel(float kernels[9], sampler2D t, vec2 uv)
{
	vec2 offsets[9] = vec2[](
		vec2(-GM_effects_texture_offset_x, -GM_effects_texture_offset_y),
		vec2( 0.0f, -GM_effects_texture_offset_y),
		vec2( GM_effects_texture_offset_x, -GM_effects_texture_offset_y),
		vec2(-GM_effects_texture_offset_x, 0.0f),  
		vec2( 0.0f, 0.0f),  
		vec2( GM_effects_texture_offset_x, 0.0f),
		vec2(-GM_effects_texture_offset_x, GM_effects_texture_offset_y),
		vec2( 0.0f, GM_effects_texture_offset_y),
		vec2( GM_effects_texture_offset_x, GM_effects_texture_offset_y)
	);
	vec3 sampler[9];
	for(int i = 0; i < 9; i++)
	{
		sampler[i] = vec3(texture(t, uv + offsets[i]));
	}
	vec3 color = vec3(0);
	for(int i = 0; i < 9; i++)
		color += sampler[i] * kernels[i];
	return color;
}

vec3 inv(sampler2D t, vec2 uv)
{
	vec3 color = texture(t, uv).rgb;
	return vec3(1.f - color.r, 1.f - color.g, 1.f - color.b);
}

vec3 sharpen(sampler2D t, vec2 uv)
{
	float kernels[9] = float[](
		-1, -1, -1,
		-1,  9, -1,
		-1, -1, -1
	);
	return kernel(kernels, t, uv);
}

vec3 blur(sampler2D t, vec2 uv)
{
	float kernels[9] = float[](
		1.0 / 16, 2.0 / 16, 1.0 / 16,
		2.0 / 16, 4.0 / 16, 2.0 / 16,
		1.0 / 16, 2.0 / 16, 1.0 / 16  
	);
	return kernel(kernels, t, uv);
}

vec3 gray(sampler2D t, vec2 uv)
{
	vec3 fragColor = texture(t, uv).rgb;
	float average = 0.2126 * fragColor.r + 0.7152 * fragColor.g + 0.0722 * fragColor.b;
	return vec3(average, average, average);
}

vec3 edgeDetect(sampler2D t, vec2 uv)
{
		float kernels[9] = float[](
		1, 1, 1,
		1, -8, 1,
		1, 1, 1
	);
	return kernel(kernels, t, uv);
}

vec3 effects(sampler2D t, vec2 uv)
{
	vec3 result = vec3(0, 0, 0);
	if (GM_filter == Filter_Inversion)
		result = inv(t, uv);
	else if (GM_filter == Filter_Sharpen)
		result = sharpen(t, uv);
	else if (GM_filter == Filter_Blur)
		result = blur(t, uv);
	else if (GM_filter == Filter_Grayscale)
		result = gray(t, uv);
	else if (GM_filter == Filter_EdgeDetect)
		result = edgeDetect(t, uv);
	else
		result = texture(t, uv).rgb;
	return result;
}
