uniform sampler2D gFramebuffer;

bool g_hasEffect = false;
float g_offset = 1 / 300.f;

// Effect switches
uniform int GM_effects_inversion;
uniform int GM_effects_sharpen;
uniform int GM_effects_blur;
uniform int GM_effects_grayscale;
uniform int GM_effects_edgedetect;

vec3 kernel(float kernels[9], sampler2D t, vec2 uv)
{
    vec2 offsets[9] = vec2[](
        vec2(-g_offset,  g_offset),
        vec2( 0.0f,    g_offset),
        vec2( g_offset,  g_offset),
        vec2(-g_offset,  0.0f),  
        vec2( 0.0f,    0.0f),  
        vec2( g_offset,  0.0f),  
        vec2(-g_offset, -g_offset),
        vec2( 0.0f,   -g_offset),
        vec2( g_offset, -g_offset) 
    );
    vec3 sampler[9];
    for(int i = 0; i < 9; i++)
    {
        sampler[i] = vec3(texture(t, uv + offsets[i]));
    }
    vec3 color;
    for(int i = 0; i < 9; i++)
        color += sampler[i] * kernels[i];
    return color;
}

bool hasEffect(int effect)
{
    if (effect == 1)
    {
        g_hasEffect = true;
        return true;
    }
    return false;
}

vec3 inv(sampler2D t, vec2 uv)
{
    vec3 color = texture(t, uv).rgb;
    return vec3(1.f - color.r, 1.f - color.g, 1.f - color.b);
}

vec3 sharpen(sampler2D t, vec2 uv)
{
    float kernel[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );
    return kernel(kernel, t, uv);
}

vec3 blur(sampler2D t, vec2 uv)
{
    float kernel[9] = float[](
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16  
    );
    return kernel(kernel, t, uv);
}

vec3 gray(sampler2D t, vec2 uv)
{
    vec3 fragColor = texture(t, uv).rgb;
    float average = 0.2126 * fragColor.r + 0.7152 * fragColor.g + 0.0722 * fragColor.b;
    return vec3(average, average, average);
}

vec3 edgeDetect(sampler2D t, vec2 uv)
{
        float kernel[9] = float[](
        1, 1, 1,
        1, -8, 1,
        1, 1, 1
    );
    return kernel(kernel, t, uv);
}

vec3 effects(sampler2D t, vec2 uv)
{
    bool hasEffect = false;
    vec3 result = vec3(0, 0, 0);
    if (hasEffect(GM_effects_inversion))
        result += inv(t, uv);
    else if (hasEffect(GM_effects_sharpen))
        result += sharpen(t, uv);
    else if (hasEffect(GM_effects_blur))
        result += blur(t, uv);
    else if (hasEffect(GM_effects_grayscale))
        result += gray(t, uv);
    else if (hasEffect(GM_effects_edgedetect))
        result += edgeDetect(t, uv);
    else if (!g_hasEffect)
        result = texture(t, uv).rgb;
    return result;
}