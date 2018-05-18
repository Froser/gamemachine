#version 410
in vec2 _texCoords;
out vec4 frag_color;
uniform sampler2D GM_framebuffer;

uniform int GM_HDR;
uniform int GM_ToneMapping;
vec4 calculateWithToneMapping(vec4 c)
{
    const int ReinhardToneMapping = 0;

    if (GM_HDR > 0)
    {
        if (GM_ToneMapping == ReinhardToneMapping)
        {
            vec3 color = c.rgb;
            return vec4(color / (color + vec3(1,1,1)), 1);
        }
    }
    return c;
}

struct GMScreenInfo
{
    int GM_Multisampling;
    int GM_ScreenWidth;
    int GM_ScreenHeight;
};
uniform GMScreenInfo GM_ScreenInfo;
uniform float GM_KernelDeltaX;
uniform float GM_KernelDeltaY;

vec3 kernel(float kernels[9], sampler2D t, vec2 uv)
{
    float X = GM_KernelDeltaX / GM_ScreenInfo.GM_ScreenWidth;
    float Y = GM_KernelDeltaY / GM_ScreenInfo.GM_ScreenHeight;

    vec2 offsets[9] = vec2[](
        vec2(-X, -Y),
        vec2( 0.0f, -Y),
        vec2( X, -Y),
        vec2(-X, 0.0f),  
        vec2( 0.0f, 0.0f),  
        vec2( X, 0.0f),
        vec2(-X, Y),
        vec2( 0.0f, Y),
        vec2( X, Y)
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

subroutine vec3 GM_FilterRoutineType(sampler2D t, vec2 texcoord);

subroutine (GM_FilterRoutineType) vec3 GM_InversionFilter(sampler2D t, vec2 uv)
{
    vec3 color = texture(t, uv).rgb;
    return vec3(1.f - color.r, 1.f - color.g, 1.f - color.b);
}

subroutine (GM_FilterRoutineType) vec3 GM_SharpenFilter(sampler2D t, vec2 uv)
{
    float kernels[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );
    return kernel(kernels, t, uv);
}

subroutine (GM_FilterRoutineType) vec3 GM_BlurFilter(sampler2D t, vec2 uv)
{
    float kernels[9] = float[](
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16  
    );
    return kernel(kernels, t, uv);
}

subroutine (GM_FilterRoutineType) vec3 GM_GrayscaleFilter(sampler2D t, vec2 uv)
{
    vec3 fragColor = texture(t, uv).rgb;
    float average = 0.2126 * fragColor.r + 0.7152 * fragColor.g + 0.0722 * fragColor.b;
    return vec3(average, average, average);
}

subroutine (GM_FilterRoutineType) vec3 GM_EdgeDetectFilter(sampler2D t, vec2 uv)
{
        float kernels[9] = float[](
        1, 1, 1,
        1, -8, 1,
        1, 1, 1
    );
    return kernel(kernels, t, uv);
}

subroutine (GM_FilterRoutineType) vec3 GM_DefaultFilter(sampler2D t, vec2 uv)
{
    return texture(t, uv).rgb;
}

subroutine uniform GM_FilterRoutineType GM_Filter;

void main()
{
    frag_color = vec4(max(GM_Filter(GM_framebuffer, _texCoords), vec3(0, 0, 0)), 1);
    frag_color = calculateWithToneMapping(frag_color);
}
