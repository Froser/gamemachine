// 位置
uniform vec4 GM_ViewPosition;

// 阴影纹理
uniform sampler2DShadow GM_shadow_texture;
uniform int GM_shadow_texture_switch = 0;

struct GMTexture
{
    sampler2D Texture;
    float OffsetX;
    float OffsetY;
    float ScaleX;
    float ScaleY;
    int Enabled;
};
uniform GMTexture GM_AmbientTextureAttribute;
uniform GMTexture GM_DiffuseTextureAttribute;
uniform GMTexture GM_SpecularTextureAttribute;
uniform GMTexture GM_LightmapTextureAttribute;
uniform GMTexture GM_NormalMapTextureAttribute;
uniform GMTexture GM_AlbedoTextureAttribute;
uniform GMTexture GM_MetallicRoughnessAOTextureAttribute;
uniform samplerCube GM_CubeMapTextureAttribute; //CubeMap作为单独一个纹理

vec4 GM_SampleTextures(GMTexture tex, vec2 uv)
{
    return tex.Enabled == 1
            ? vec4(texture(tex.Texture, uv * vec2(tex.ScaleX, tex.ScaleY) + vec2(tex.OffsetX, tex.OffsetY)))
            : vec4(0);
}

struct GMMaterial
{
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float Shininess;
    float Refractivity;
    vec3 F0;
};
uniform GMMaterial GM_Material;
