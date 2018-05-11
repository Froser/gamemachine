//--------------------------------------------------------------------------------------
// Functions
//--------------------------------------------------------------------------------------
float4 ToFloat4(float3 v, float w)
{
    return float4(v.x, v.y, v.z, w);
}

float4 ToFloat4(float3 v)
{
    return ToFloat4(v, 1);
}

float3x3 ToFloat3x3(float4x4 m)
{
    return float3x3(
        m[0].xyz,
        m[1].xyz,
        m[2].xyz
    );
}

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer WorldConstantBuffer: register( b0 ) 
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix InverseTransposeModelMatrix;
    matrix InverseViewMatrix;
    float4 ViewPosition;
}

//--------------------------------------------------------------------------------------
// Textures, LightAttributes, Materials
//--------------------------------------------------------------------------------------
Texture2D AmbientTexture_0: register(t0);
Texture2D AmbientTexture_1: register(t1);
Texture2D AmbientTexture_2: register(t2);
Texture2D DiffuseTexture_0: register(t3);
Texture2D DiffuseTexture_1: register(t4);
Texture2D DiffuseTexture_2: register(t5);
Texture2D SpecularTexture_0: register(t6);
Texture2D SpecularTexture_1: register(t7);
Texture2D SpecularTexture_2: register(t8);
Texture2D NormalMapTexture: register(t9);
Texture2D LightmapTexture: register(t10);
TextureCube CubeMapTexture: register(t11);

SamplerState AmbientSampler_0: register(s0);
SamplerState AmbientSampler_1: register(s1);
SamplerState AmbientSampler_2: register(s2);
SamplerState DiffuseSampler_0: register(s3);
SamplerState DiffuseSampler_1: register(s4);
SamplerState DiffuseSampler_2: register(s5);
SamplerState SpecularSampler_0: register(s6);
SamplerState SpecularSampler_1: register(s7);
SamplerState SpecularSampler_2: register(s8);
SamplerState NormalMapSampler: register(s9);
SamplerState LightmapSampler: register(s10);
SamplerState CubeMapSampler: register(s11);

SamplerState ShadowMapSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = BORDER;
    AddressV = BORDER;
    BorderColor = float4(1, 1, 1, 1);
};

float3 TextureRGBToNormal(Texture2D tex, SamplerState ss, float2 texcoord)
{
    return tex.Sample(ss, texcoord).xyz * 2.0f - 1.0f;
}

float3 TextureRGBToNormal(Texture2DMS<float4> tex, int3 coord)
{
    return tex.Load(coord, 0).xyz * 2.0f - 1.0f;
}

float3 TextureRGBToNormal(Texture2D tex, int3 coord)
{
    return tex.Load(coord).xyz * 2.0f - 1.0f;
}

float3 RGBToNormal(float3 rgb)
{
    return rgb * 2.0f - 1.0f;
}

struct GMMaterial
{
    float4 Ka;
    float4 Kd;
    float4 Ks;
    float Shininess;
    float Refractivity;
};
GMMaterial Material;

class GMTexture
{
    float OffsetX;
    float OffsetY;
    float ScaleX;
    float ScaleY;
    bool Enabled;

    float4 Sample(Texture2D tex, SamplerState ss, float2 texcoord)
    {
        if (!Enabled)
            return float4(0.0f, 0.0f, 0.0f, 0.0f);

        float2 transformedTexcoord = texcoord * float2(ScaleX, ScaleY) + float2(OffsetX, OffsetY);
        return tex.Sample(ss, transformedTexcoord);
    }

    float3 RGBToNormal(Texture2D tex, SamplerState ss, float2 texcoord)
    {
        return TextureRGBToNormal(tex, ss, texcoord);
    }
};

class GMLightmapTexture : GMTexture
{
    float4 Sample(Texture2D tex, SamplerState ss, float2 texcoord)
    {
        if (!Enabled)
            return float4(1.0f, 1.0f, 1.0f, 1.0f);

        float2 transformedTexcoord = texcoord * float2(ScaleX, ScaleY) + float2(OffsetX, OffsetY);
        return tex.Sample(ss, transformedTexcoord);
    }
};

class GMCubeMapTexture : GMTexture
{
    float4 Sample(TextureCube tex, SamplerState ss, float3 texcoord)
    {
        if (!Enabled)
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
        return tex.Sample(ss, texcoord);
    }
};

GMTexture AmbientTextureAttributes[3];
GMTexture DiffuseTextureAttributes[3];
GMTexture SpecularTextureAttributes[3];
GMTexture NormalMapTextureAttributes[1];
GMLightmapTexture LightmapTextureAttributes[1];
GMCubeMapTexture CubeMapTextureAttributes[1];

interface ILight
{
    float4 IlluminateAmbient();
    float4 IlluminateDiffuse(float3 lightDirection_N, float3 normal_N);
    float4 IlluminateSpecular(float3 lightDirection_N, float3 eyeDirection_N, float3 normal_N, float shininess);
};

float3 GetLightPositionInEyeSpace(float4 pos, float4x4 viewMatrix)
{
    return (mul(pos, viewMatrix)).xyz;
}

class GMDefaultAmbientLight : ILight
{
    float4 IlluminateAmbient()
    {
        return 1;
    }

    float4 IlluminateDiffuse(float3 lightDirection_N, float3 normal_N)
    {
        return 0;
    }

    float4 IlluminateSpecular(float3 lightDirection_N, float3 eyeDirection_N, float3 normal_N, float shininess)
    {
        return 0;
    }
};

class GMDefaultDirectLight : ILight
{
    float4 IlluminateAmbient()
    {
        return 0;
    }

    float4 IlluminateDiffuse(float3 lightDirection_N, float3 normal_N)
    {
        return saturate(dot(lightDirection_N, normal_N));
    }

    float4 IlluminateSpecular(float3 lightDirection_N, float3 eyeDirection_N, float3 normal_N, float shininess)
    {
        float3 reflection_N = reflect(-lightDirection_N, normal_N);
        float theta = dot(eyeDirection_N, reflection_N);
        float factor_Specular = saturate(pow(max(theta, 0), shininess));
        return factor_Specular;
    }
};

//默认的光照实现
GMDefaultAmbientLight DefaultAmbientLight; 
GMDefaultDirectLight DefaultDirectLight;

struct GMLight
{
    float4 Position;
    float4 Color;
    int Type;
};
const int GM_AmbientLight = 0;
const int GM_DirectLight = 1;

class GMLightProxy
{
    float4 IlluminateAmbient(GMLight light)
    {
        if (light.Type == GM_AmbientLight)
            return DefaultAmbientLight.IlluminateAmbient();

        return DefaultDirectLight.IlluminateAmbient();
    }

    float4 IlluminateDiffuse(GMLight light, float3 lightDirection_N, float3 normal_N)
    {
        if (light.Type == GM_AmbientLight)
            return DefaultAmbientLight.IlluminateDiffuse(lightDirection_N, normal_N);

        return DefaultDirectLight.IlluminateDiffuse(lightDirection_N, normal_N);
    }

    float4 IlluminateSpecular(GMLight light, float3 lightDirection_N, float3 eyeDirection_N, float3 normal_N, float shininess)
    {
        if (light.Type == GM_AmbientLight)
            return DefaultAmbientLight.IlluminateSpecular(lightDirection_N, eyeDirection_N, normal_N, shininess);

        return DefaultDirectLight.IlluminateSpecular(lightDirection_N, eyeDirection_N, normal_N, shininess);
    }
};

GMLight LightAttributes[50];
int LightCount;
GMLightProxy LightProxy;

struct GMScreenInfo
{
    int ScreenWidth;
    int ScreenHeight;
    bool Multisampling;
};

GMScreenInfo ScreenInfo;

//--------------------------------------------------------------------------------------
// Gamma Correction
//--------------------------------------------------------------------------------------
bool GammaCorrection;
float Gamma;
float4 CalculateGammaCorrection(float4 factor)
{
    if (!GammaCorrection)
        return factor;

    float inverseGamma = 1.0f / Gamma;
    return ToFloat4(pow(factor.rgb, float3(inverseGamma, inverseGamma, inverseGamma)));
}

//--------------------------------------------------------------------------------------
// Shadow
//--------------------------------------------------------------------------------------
struct GMShadowInfo
{
    bool HasShadow;
    matrix ShadowMatrix;
    float4 Position;
    int ShadowMapWidth;
    int ShadowMapHeight;
    float BiasMin;
    float BiasMax;
};
Texture2D ShadowMap;
Texture2DMS<float4> ShadowMapMSAA;

GMShadowInfo ShadowInfo;
//--------------------------------------------------------------------------------------
// States
//--------------------------------------------------------------------------------------
RasterizerState GMRasterizerState {};
BlendState GMBlendState {};
DepthStencilState GMDepthStencilState {};
//--------------------------------------------------------------------------------------

struct VS_INPUT
{
    float3 Position    : POSITION;
    float3 Normal      : NORMAL0;
    float2 Texcoord    : TEXCOORD0;
    float3 Tangent     : NORMAL1;
    float3 Bitangent   : NORMAL2;
    float2 Lightmap    : TEXCOORD1;
    float4 Color       : COLOR;
};

struct VS_OUTPUT
{
    float3 Normal      : NORMAL0;
    float2 Texcoord    : TEXCOORD0;
    float3 Tangent     : NORMAL1;
    float3 Bitangent   : NORMAL2;
    float2 Lightmap    : TEXCOORD1;
    float4 Color       : COLOR;
    float4 WorldPos    : POSITION;
    float4 Position    : SV_POSITION;
};

typedef VS_OUTPUT PS_INPUT;

bool HasNoTexture(GMTexture attributes[3])
{
    for (int i = 0; i < 3; ++i)
    {
        if (attributes[i].Enabled)
            return false;
    }
    return true;
}

class TangentSpace
{
    float3 Normal_Tangent_N;
    float3x3 TBN;

    void CalculateTangentSpaceInEyeSpace(
        float2 texcoord,
        float3 tangent,
        float3 bitangent,
        float3 normal_Eye_N,
        float3x3 transform_Normal_Eye,
        GMTexture normalMap
        )
    {
        Normal_Tangent_N = normalMap.RGBToNormal(NormalMapTexture, NormalMapSampler, texcoord);
        float3 tangent_Eye_N = normalize(mul(tangent, transform_Normal_Eye).xyz);
        float3 bitangent_Eye_N = normalize(mul(bitangent, transform_Normal_Eye).xyz);
        TBN = transpose(float3x3(
            tangent_Eye_N,
            bitangent_Eye_N,
            normal_Eye_N
        ));
    }
};

float4 IlluminateRefractionByNormalWorldN(
    GMCubeMapTexture cubeMap,
    TextureCube tex,
    float3 normal_World_N,
    float4 position_World,
    float4 viewPosition_World,
    float refractivity
    )
{
    float3 in_N = normalize((position_World - viewPosition_World).xyz);
    float3 refraction = refract(in_N, normal_World_N, refractivity);
    return cubeMap.Sample(tex, CubeMapSampler, refraction.xyz);
}

float4 IlluminateRefraction(
    GMCubeMapTexture cubeMap,
    TextureCube tex,
    float3 normal_World_N,
    float4 position_World,
    float4 viewPosition_World,
    bool hasNormalMap,
    TangentSpace tangentSpace,
    float refractivity
    )
{
    if (refractivity == 0)
        return float4(0, 0, 0, 0);

    if (hasNormalMap)
    {
        // 如果是切线空间，计算会复杂点，要将切线空间的法线换算回世界空间
        float3 normal_Eye_N = mul(tangentSpace.Normal_Tangent_N, transpose(tangentSpace.TBN));
        float3 normalFromTangent_World_N = mul(normal_Eye_N, ToFloat3x3(InverseViewMatrix));
        return IlluminateRefractionByNormalWorldN(cubeMap, tex, normalFromTangent_World_N, position_World, viewPosition_World, refractivity);
    }

    return IlluminateRefractionByNormalWorldN(cubeMap, tex, normal_World_N, position_World, viewPosition_World, refractivity);
}

//--------------------------------------------------------------------------------------
// 3D
//--------------------------------------------------------------------------------------

struct PS_3D_INPUT
{
    float3 WorldPos;            // 世界坐标
    float3 Normal_World_N;      // 世界法线
    float3 Normal_Eye_N;        // 眼睛空间法向量
    TangentSpace TangentSpace;  // 切线空间
    bool HasNormalMap;          // 是否有法线贴图
    float3 AmbientLightmapTexture;
    float3 DiffuseTexture;
    float3 SpecularTexture;
    float Shininess;
    float Refractivity; 
};

float CalculateShadow(matrix shadowMatrix, float4 worldPos, float3 normal_N)
{
    if (!ShadowInfo.HasShadow)
        return 1.0f;

    float4 fragPos = mul(worldPos, shadowMatrix);
    float3 projCoords = fragPos.xyz / fragPos.w;
    if (projCoords.z > 1.0f)
        return 1.0f;

    projCoords.x = projCoords.x * 0.5f + 0.5f;
    projCoords.y = projCoords.y * (-0.5f) + 0.5f;

    float bias = (ShadowInfo.BiasMin == ShadowInfo.BiasMax) ? ShadowInfo.BiasMin : max(ShadowInfo.BiasMax * (1.0 - dot(normal_N, normalize(worldPos.xyz - ShadowInfo.Position.xyz))), ShadowInfo.BiasMin);
    float closestDepth = 0;
    if (ScreenInfo.Multisampling)
    {
        int x = ShadowInfo.ShadowMapWidth * projCoords.x;
        int y = ShadowInfo.ShadowMapHeight * projCoords.y;
        if (projCoords.x > 1 || projCoords.x < 0 ||
            projCoords.y > 1 || projCoords.y < 0 )
        {
            return 1.f;
        }

        closestDepth = ShadowMapMSAA.Load(int3(x, y, 0), 0);
    }
    else
    {
        closestDepth = ShadowMap.Sample(ShadowMapSampler, projCoords.xy).r;
    }

    return projCoords.z - bias > closestDepth ? 0.f : 1.f;
}

float4 PS_3D_CalculateColor(PS_3D_INPUT input)
{
    float factor_Shadow = CalculateShadow(ShadowInfo.ShadowMatrix, ToFloat4(input.WorldPos), input.Normal_World_N);
    float4 factor_Ambient = float4(0, 0, 0, 0);
    float4 factor_Diffuse = float4(0, 0, 0, 0);
    float4 factor_Specular = float4(0, 0, 0, 0);

    // 将法线换算到眼睛坐标系
    float3 position_Eye = (mul(ToFloat4(input.WorldPos), ViewMatrix)).xyz;
    float3 position_Eye_N = normalize(position_Eye);

    for (int i = 0; i < LightCount; ++i)
    {
        float3 lightPosition_Eye = GetLightPositionInEyeSpace(LightAttributes[i].Position, ViewMatrix);
        factor_Ambient += LightProxy.IlluminateAmbient(LightAttributes[i]) * LightAttributes[i].Color;

        if (!input.HasNormalMap)
        {
            float3 lightDirection_Eye_N = normalize(lightPosition_Eye - position_Eye);
            factor_Diffuse += LightProxy.IlluminateDiffuse(LightAttributes[i], lightDirection_Eye_N, input.Normal_Eye_N) * LightAttributes[i].Color;
            factor_Specular += LightProxy.IlluminateSpecular(LightAttributes[i], lightDirection_Eye_N, -position_Eye_N, input.Normal_Eye_N, input.Shininess) * LightAttributes[i].Color;
        }
        else
        {
            float3 lightDirection_Eye = lightPosition_Eye - position_Eye;
            float3 lightDirection_Tangent_N = normalize(mul(lightDirection_Eye, input.TangentSpace.TBN));
            float3 eyeDirection_Tangent_N = normalize(mul(-position_Eye, input.TangentSpace.TBN));
            factor_Diffuse += LightProxy.IlluminateDiffuse(LightAttributes[i], lightDirection_Tangent_N, input.TangentSpace.Normal_Tangent_N) * LightAttributes[i].Color;
            factor_Specular += LightProxy.IlluminateSpecular(LightAttributes[i], lightDirection_Tangent_N, eyeDirection_Tangent_N, input.TangentSpace.Normal_Tangent_N, input.Shininess) * LightAttributes[i].Color; 
        }
    }
    float4 color_Ambient = CalculateGammaCorrection(factor_Ambient) * saturate(ToFloat4(input.AmbientLightmapTexture));
    float4 color_Diffuse = factor_Shadow * CalculateGammaCorrection(factor_Diffuse) * saturate(ToFloat4(input.DiffuseTexture));

    // 计算Specular
    float4 color_Specular = factor_Shadow * CalculateGammaCorrection(factor_Specular) * ToFloat4(input.SpecularTexture);
    
    // 计算折射
    float4 color_Refractivity = IlluminateRefraction(
        CubeMapTextureAttributes[0],
        CubeMapTexture,
        input.Normal_World_N,
        ToFloat4(input.WorldPos),
        ViewPosition,
        input.HasNormalMap,
        input.TangentSpace,
        input.Refractivity
        );
    color_Refractivity = color_Refractivity;

    float4 finalColor = saturate(color_Ambient + color_Diffuse + color_Specular + color_Refractivity);
    return finalColor;
}

VS_OUTPUT VS_3D( VS_INPUT input )
{
    VS_OUTPUT output;
    output.Position = ToFloat4(input.Position);
    output.Position = mul(output.Position, WorldMatrix);
    output.WorldPos = output.Position;
    
    output.Position = mul(output.Position, ViewMatrix);
    output.Position = mul(output.Position, ProjectionMatrix);

    output.Normal = input.Normal;
    output.Texcoord = input.Texcoord;
    output.Tangent = input.Tangent;
    output.Bitangent = input.Bitangent;
    output.Lightmap = input.Lightmap;
    output.Color = input.Color;
    return output;
}

bool PS_3D_HasNormalMap()
{
    return NormalMapTextureAttributes[0].Enabled;
}

GMTexture PS_3D_NormalMap()
{
    return NormalMapTextureAttributes[0];
}

float4 PS_3D(PS_INPUT input) : SV_TARGET
{
    // 将法线换算到眼睛坐标系
    float3x3 inverseTransposeModelMatrix = ToFloat3x3(InverseTransposeModelMatrix);
    float3x3 transform_Normal_Eye = mul(inverseTransposeModelMatrix, ToFloat3x3(ViewMatrix));
    float3 normal_Eye_N = normalize(mul(input.Normal, transform_Normal_Eye));

    PS_3D_INPUT commonInput;
    TangentSpace tangentSpace;
    if (PS_3D_HasNormalMap())
        tangentSpace.CalculateTangentSpaceInEyeSpace(input.Texcoord, input.Tangent, input.Bitangent, normal_Eye_N, transform_Normal_Eye, PS_3D_NormalMap());

    commonInput.TangentSpace = tangentSpace;
    commonInput.WorldPos = input.WorldPos;
    commonInput.HasNormalMap = PS_3D_HasNormalMap();

    commonInput.Normal_World_N = normalize(mul(input.Normal, inverseTransposeModelMatrix));
    commonInput.Normal_Eye_N = normal_Eye_N;

    // 计算Ambient
    float4 color_Ambient = float4(0, 0, 0, 0);
    if (HasNoTexture(AmbientTextureAttributes))
    {
        color_Ambient = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
        color_Ambient += AmbientTextureAttributes[0].Sample(AmbientTexture_0, AmbientSampler_0, input.Texcoord);
        color_Ambient += AmbientTextureAttributes[1].Sample(AmbientTexture_1, AmbientSampler_1, input.Texcoord);
        color_Ambient += AmbientTextureAttributes[2].Sample(AmbientTexture_2, AmbientSampler_2, input.Texcoord);
    }
    color_Ambient *= LightmapTextureAttributes[0].Sample(LightmapTexture, LightmapSampler, input.Lightmap);

    // 计算Diffuse
    float4 color_Diffuse = float4(0, 0, 0, 0);
    if (HasNoTexture(DiffuseTextureAttributes))
    {
        color_Diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
        color_Diffuse += DiffuseTextureAttributes[0].Sample(DiffuseTexture_0, DiffuseSampler_0, input.Texcoord);
        color_Diffuse += DiffuseTextureAttributes[1].Sample(DiffuseTexture_1, DiffuseSampler_1, input.Texcoord);
        color_Diffuse += DiffuseTextureAttributes[2].Sample(DiffuseTexture_2, DiffuseSampler_2, input.Texcoord);
    }

    // 计算Specular(如果有Specular贴图)
    float4 color_Specular = 0;
    if (HasNoTexture(SpecularTextureAttributes))
    {
        color_Specular = 1.f;
    }
    else
    {
        color_Specular += SpecularTextureAttributes[0].Sample(SpecularTexture_0, SpecularSampler_0, input.Texcoord).r;
        color_Specular += SpecularTextureAttributes[1].Sample(SpecularTexture_1, SpecularSampler_1, input.Texcoord).r;
        color_Specular += SpecularTextureAttributes[2].Sample(SpecularTexture_2, SpecularSampler_2, input.Texcoord).r;
    }

    commonInput.AmbientLightmapTexture = color_Ambient * Material.Ka;
    commonInput.DiffuseTexture = color_Diffuse * Material.Kd;
    commonInput.SpecularTexture = color_Specular * Material.Ks;
    commonInput.Shininess = Material.Shininess;
    commonInput.Refractivity = Material.Refractivity;

    return PS_3D_CalculateColor(commonInput);
}

//--------------------------------------------------------------------------------------
// 2D
//--------------------------------------------------------------------------------------
VS_OUTPUT VS_Flat(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = float4(input.Position.x, input.Position.y, input.Position.z, 1);
    output.Position = mul(output.Position, WorldMatrix);
    output.Position.z = 0;
    output.Normal = input.Normal;
    output.Texcoord = input.Texcoord;
    output.Tangent = input.Tangent;
    output.Bitangent = input.Bitangent;
    output.Lightmap = input.Lightmap;
    output.Color = input.Color;
    return output;
}

VS_OUTPUT VS_2D(VS_INPUT input)
{
    return VS_Flat(input);
}

float4 PS_2D(PS_INPUT input) : SV_TARGET
{
    if (HasNoTexture(DiffuseTextureAttributes) && HasNoTexture(AmbientTextureAttributes))
        return float4(0, 0, 0, 0);

    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    color += AmbientTextureAttributes[0].Sample(AmbientTexture_0, AmbientSampler_0, input.Texcoord);
    color += AmbientTextureAttributes[1].Sample(AmbientTexture_1, AmbientSampler_1, input.Texcoord);
    color += AmbientTextureAttributes[2].Sample(AmbientTexture_2, AmbientSampler_2, input.Texcoord);
    color += DiffuseTextureAttributes[0].Sample(DiffuseTexture_0, DiffuseSampler_0, input.Texcoord);
    color += DiffuseTextureAttributes[1].Sample(DiffuseTexture_1, DiffuseSampler_1, input.Texcoord);
    color += DiffuseTextureAttributes[2].Sample(DiffuseTexture_2, DiffuseSampler_2, input.Texcoord);
    return color;
}

//--------------------------------------------------------------------------------------
// Glyph
//--------------------------------------------------------------------------------------
VS_OUTPUT VS_Glyph(VS_INPUT input)
{
    return VS_Flat(input);
}

float4 PS_Glyph(PS_INPUT input) : SV_TARGET
{
    float4 alpha = AmbientTexture_0.Sample(AmbientSampler_0, input.Texcoord);
    return float4(input.Color.r, input.Color.g, input.Color.b, alpha.r);
}

//--------------------------------------------------------------------------------------
// CubeMap
//--------------------------------------------------------------------------------------
VS_OUTPUT VS_CubeMap(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = float4(input.Position.x, input.Position.y, input.Position.z, 1);
    output.WorldPos = output.Position;
    output.Position = mul(output.Position, WorldMatrix);
    output.Position = mul(output.Position, ViewMatrix);
    output.Position = mul(output.Position, ProjectionMatrix);
    return output;
}

float4 PS_CubeMap(PS_INPUT input) : SV_TARGET
{
    float3 texcoord = input.WorldPos.xyz;
    return CubeMapTextureAttributes[0].Sample(CubeMapTexture, CubeMapSampler, texcoord);
}

//--------------------------------------------------------------------------------------
// Deferred 3D
//--------------------------------------------------------------------------------------
Texture2D GM_DeferredPosition_World_Refractivity;
Texture2D GM_DeferredNormal_World;
Texture2D GM_DeferredTextureAmbient;
Texture2D GM_DeferredTextureDiffuse;
Texture2D GM_DeferredTangent_Eye;
Texture2D GM_DeferredBitangent_Eye;
Texture2D GM_DeferredNormalMap_bNormalMap;
Texture2D GM_DeferredKs_Shininess;

Texture2DMS<float4> GM_DeferredPosition_World_Refractivity_MSAA;
Texture2DMS<float4> GM_DeferredNormal_World_MSAA;
Texture2DMS<float4> GM_DeferredTextureAmbient_MSAA;
Texture2DMS<float4> GM_DeferredTextureDiffuse_MSAA;
Texture2DMS<float4> GM_DeferredTangent_Eye_MSAA;
Texture2DMS<float4> GM_DeferredBitangent_Eye_MSAA;
Texture2DMS<float4> GM_DeferredNormalMap_bNormalMap_MSAA;
Texture2DMS<float4> GM_DeferredKs_Shininess_MSAA;

// [-1, 1] -> [0, 1]
float4 Float3ToTexture(float3 normal)
{
    return ToFloat4((normal + 1) * .5f, 1);
}

struct VS_GEOMETRY_OUTPUT
{
    float4 Position_Refractivity             : SV_TARGET0;
    float4 Normal_World                      : SV_TARGET1; //需要将区间转化到[0, 1]
    float4 TextureAmbient                    : SV_TARGET2;
    float4 TextureDiffuse                    : SV_TARGET3;
    float4 Tangent_Eye                       : SV_TARGET4; //需要将区间转化到[0, 1]
    float4 Bitangent_Eye                     : SV_TARGET5; //需要将区间转化到[0, 1]
    float4 NormalMap_HasNormalMap            : SV_TARGET6;
    float4 Ks_Shininess                      : SV_TARGET7;
};

VS_OUTPUT VS_3D_GeometryPass(VS_INPUT input)
{
    return VS_3D(input);
}

VS_GEOMETRY_OUTPUT PS_3D_GeometryPass(PS_INPUT input)
{
    VS_GEOMETRY_OUTPUT output;
    output.Position_Refractivity.rgb = input.WorldPos;
    output.Position_Refractivity.a = Material.Refractivity;

    float3x3 inverseTransposeModelMatrix = ToFloat3x3(InverseTransposeModelMatrix);
    float3x3 normalEyeTransform = mul(inverseTransposeModelMatrix, ToFloat3x3(ViewMatrix));
    float4 normal_Model = ToFloat4(input.Normal.xyz, 0);
    output.Normal_World = Float3ToTexture( normalize(mul(input.Normal.xyz, inverseTransposeModelMatrix)) );

    float4 texAmbient = float4(0, 0, 0, 0);
    float4 texDiffuse = float4(0, 0, 0, 0);
    if (HasNoTexture(AmbientTextureAttributes))
    {
        texAmbient = float4(1, 1, 1, 1);
    }
    else
    {
        texAmbient += AmbientTextureAttributes[0].Sample(AmbientTexture_0, AmbientSampler_0, input.Texcoord);
        texAmbient += AmbientTextureAttributes[1].Sample(AmbientTexture_1, AmbientSampler_1, input.Texcoord);
        texAmbient += AmbientTextureAttributes[2].Sample(AmbientTexture_2, AmbientSampler_2, input.Texcoord);
        texAmbient *= LightmapTextureAttributes[0].Sample(LightmapTexture, LightmapSampler, input.Lightmap);
    }
    if (HasNoTexture(DiffuseTextureAttributes))
    {
        texDiffuse = float4(1, 1, 1, 1);
    }
    else
    {
        texDiffuse += DiffuseTextureAttributes[0].Sample(DiffuseTexture_0, DiffuseSampler_0, input.Texcoord);
        texDiffuse += DiffuseTextureAttributes[1].Sample(DiffuseTexture_1, DiffuseSampler_1, input.Texcoord);
        texDiffuse += DiffuseTextureAttributes[2].Sample(DiffuseTexture_2, DiffuseSampler_2, input.Texcoord);
    }
    output.TextureAmbient = texAmbient * Material.Ka;
    output.TextureDiffuse = texDiffuse * Material.Kd;

    float texSpecular = 0;
    if (HasNoTexture(SpecularTextureAttributes))
    {
        texSpecular = 1.f;
    }
    else
    {
        texSpecular += SpecularTextureAttributes[0].Sample(SpecularTexture_0, SpecularSampler_0, input.Texcoord).r;
        texSpecular += SpecularTextureAttributes[1].Sample(SpecularTexture_1, SpecularSampler_1, input.Texcoord).r;
        texSpecular += SpecularTextureAttributes[2].Sample(SpecularTexture_2, SpecularSampler_2, input.Texcoord).r;
    }

    if (PS_3D_HasNormalMap())
    {
        output.Tangent_Eye = Float3ToTexture(normalize(mul(input.Tangent, normalEyeTransform)));
        output.Bitangent_Eye = Float3ToTexture(normalize(mul(input.Bitangent, normalEyeTransform)));
        output.NormalMap_HasNormalMap = ToFloat4(PS_3D_NormalMap().Sample(NormalMapTexture, NormalMapSampler, input.Texcoord), 1);
    }
    else
    {
        output.Tangent_Eye = Float3ToTexture(float4(0, 0, 0, 0));
        output.Bitangent_Eye = Float3ToTexture(float4(0, 0, 0, 0));
        output.NormalMap_HasNormalMap = float4(0, 0, 0, 0);
    }

    output.Ks_Shininess = ToFloat4(Material.Ks * texSpecular, Material.Shininess);
    return output;
}

VS_OUTPUT VS_3D_LightPass(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = ToFloat4(input.Position);
    output.Texcoord = input.Texcoord;
    return output;
}

float4 PS_3D_LightPass(PS_INPUT input) : SV_TARGET
{
    PS_3D_INPUT commonInput;
    int3 coord = int3(input.Texcoord * float2(ScreenInfo.ScreenWidth, ScreenInfo.ScreenHeight), 0);
    TangentSpace tangentSpace;
    float3 tangent_Eye_N;
    float3 bitangent_Eye_N;
    float4 PosRef;
    float4 KSS;
    float4 normalMapFlag;
    if (!ScreenInfo.Multisampling)
    {
        tangent_Eye_N = TextureRGBToNormal(GM_DeferredTangent_Eye, coord).rgb;
        bitangent_Eye_N = TextureRGBToNormal(GM_DeferredBitangent_Eye, coord).rgb;
        PosRef = GM_DeferredPosition_World_Refractivity.Load(coord);
        commonInput.Normal_World_N = TextureRGBToNormal(GM_DeferredNormal_World, coord);
        commonInput.AmbientLightmapTexture = (GM_DeferredTextureAmbient.Load(coord)).rgb;
        commonInput.DiffuseTexture = (GM_DeferredTextureDiffuse.Load(coord)).rgb;
        KSS = GM_DeferredKs_Shininess.Load(coord);
        normalMapFlag = GM_DeferredNormalMap_bNormalMap.Load(coord);
    }
    else
    {
        tangent_Eye_N = TextureRGBToNormal(GM_DeferredTangent_Eye_MSAA, coord).rgb;
        bitangent_Eye_N = TextureRGBToNormal(GM_DeferredBitangent_Eye_MSAA, coord).rgb;
        PosRef = GM_DeferredPosition_World_Refractivity_MSAA.Load(coord, 0);
        commonInput.Normal_World_N = TextureRGBToNormal(GM_DeferredNormal_World_MSAA, coord);
        commonInput.AmbientLightmapTexture = (GM_DeferredTextureAmbient_MSAA.Load(coord, 0)).rgb;
        commonInput.DiffuseTexture = (GM_DeferredTextureDiffuse_MSAA.Load(coord, 0)).rgb;
        KSS = GM_DeferredKs_Shininess_MSAA.Load(coord, 0);
        normalMapFlag = GM_DeferredNormalMap_bNormalMap_MSAA.Load(coord, 0);
    }

    commonInput.SpecularTexture = KSS.rgb;
    commonInput.Shininess = KSS.a;
    commonInput.WorldPos = PosRef.rgb;
    commonInput.Refractivity = PosRef.a;
    commonInput.HasNormalMap = (normalMapFlag.a != 0);
    commonInput.Normal_Eye_N = mul(commonInput.Normal_World_N, ToFloat3x3(ViewMatrix));
    tangentSpace.Normal_Tangent_N = RGBToNormal(normalMapFlag.rgb);
    tangentSpace.TBN = transpose(float3x3(
        tangent_Eye_N,
        bitangent_Eye_N,
        commonInput.Normal_Eye_N
    ));
    commonInput.TangentSpace = tangentSpace;

    return PS_3D_CalculateColor(commonInput);
}

//--------------------------------------------------------------------------------------
// Shadow
//--------------------------------------------------------------------------------------
VS_OUTPUT VS_Shadow( VS_INPUT input )
{
    VS_OUTPUT output;
    output.Position = ToFloat4(input.Position);
    output.Position = mul(output.Position, WorldMatrix);
    output.WorldPos = output.Position;
    
    output.Position = mul(output.Position, ShadowInfo.ShadowMatrix);

    output.Normal = input.Normal;
    output.Texcoord = input.Texcoord;
    output.Tangent = input.Tangent;
    output.Bitangent = input.Bitangent;
    output.Lightmap = input.Lightmap;
    output.Color = input.Color;
    return output;
}

//--------------------------------------------------------------------------------------
// Filter
//--------------------------------------------------------------------------------------
int KernelDeltaX = 0, KernelDeltaY = 0;
typedef float GMKernel[9];
interface IFilter
{
    float4 Sample(Texture2D tex, int3 coord);
    float4 SampleMSAA(Texture2DMS<float4> tex, int3 coord, int samplerId);
};

float4 Kernel(GMKernel kernel, Texture2D tex, int3 coord)
{
    int offset = 1;
    float4 sample[9];
    sample[0] = kernel[0] * tex.Load(int3(coord.x - KernelDeltaX, coord.y - KernelDeltaY, 0));
    sample[1] = kernel[1] * tex.Load(int3(coord.x, coord.y - KernelDeltaY, 0));
    sample[2] = kernel[2] * tex.Load(int3(coord.x + KernelDeltaX, coord.y - KernelDeltaY, 0));
    sample[3] = kernel[3] * tex.Load(int3(coord.x - KernelDeltaX, coord.y, 0));
    sample[4] = kernel[4] * tex.Load(int3(coord.x, coord.y, 0));
    sample[5] = kernel[5] * tex.Load(int3(coord.x + KernelDeltaX, coord.y, 0));
    sample[6] = kernel[6] * tex.Load(int3(coord.x - KernelDeltaX, coord.y + KernelDeltaY, 0));
    sample[7] = kernel[7] * tex.Load(int3(coord.x, coord.y + KernelDeltaY, 0));
    sample[8] = kernel[8] * tex.Load(int3(coord.x + KernelDeltaX, coord.y + KernelDeltaY, 0));

    return (sample[0] +
            sample[1] +
            sample[2] +
            sample[3] +
            sample[4] +
            sample[5] +
            sample[6] +
            sample[7] +
            sample[8]);
}

float4 Kernel(GMKernel kernel, Texture2DMS<float4> tex, int3 coord, int samplerId)
{
    int offset = 1;
    float4 sample[9];
    sample[0] = kernel[0] * tex.Load(int3(coord.x - KernelDeltaX, coord.y - KernelDeltaY, 0), samplerId);
    sample[1] = kernel[1] * tex.Load(int3(coord.x, coord.y - KernelDeltaY, 0), samplerId);
    sample[2] = kernel[2] * tex.Load(int3(coord.x + KernelDeltaX, coord.y - KernelDeltaY, 0), samplerId);
    sample[3] = kernel[3] * tex.Load(int3(coord.x - KernelDeltaX, coord.y, 0), samplerId);
    sample[4] = kernel[4] * tex.Load(int3(coord.x, coord.y, 0), samplerId);
    sample[5] = kernel[5] * tex.Load(int3(coord.x + KernelDeltaX, coord.y, 0), samplerId);
    sample[6] = kernel[6] * tex.Load(int3(coord.x - KernelDeltaX, coord.y + KernelDeltaY, 0), samplerId);
    sample[7] = kernel[7] * tex.Load(int3(coord.x, coord.y + KernelDeltaY, 0), samplerId);
    sample[8] = kernel[8] * tex.Load(int3(coord.x + KernelDeltaX, coord.y + KernelDeltaY, 0), samplerId);

    return (sample[0] +
            sample[1] +
            sample[2] +
            sample[3] +
            sample[4] +
            sample[5] +
            sample[6] +
            sample[7] +
            sample[8]);
}

class GMDefaultFilter : IFilter
{
    float4 Sample(Texture2D tex, int3 coord)
    {
        return tex.Load(coord);
    }

    float4 SampleMSAA(Texture2DMS<float4> tex, int3 coord, int samplerId)
    {
        return tex.Load(coord, samplerId);
    }
};

class GMInversionFilter : IFilter
{
    float4 Sample(Texture2D tex, int3 coord)
    {
        return ToFloat4((float3(1.f, 1.f, 1.f) - tex.Load(coord)).rgb, 1);
    }

    float4 SampleMSAA(Texture2DMS<float4> tex, int3 coord, int samplerId)
    {
        return ToFloat4((float3(1.f, 1.f, 1.f) - tex.Load(coord, samplerId)).rgb, 1);
    }
};

class GMSharpenFilter : IFilter
{
    float4 Sample(Texture2D tex, int3 coord)
    {
        GMKernel kernel = {
            -1, -1, -1,
            -1,  9, -1,
            -1, -1, -1
        };
        return Kernel(kernel, tex, coord);
    }

    float4 SampleMSAA(Texture2DMS<float4> tex, int3 coord, int samplerId)
    {
        GMKernel kernel = {
            -1, -1, -1,
            -1,  9, -1,
            -1, -1, -1
        };
        return Kernel(kernel, tex, coord, samplerId);
    }
};

class GMBlurFilter : IFilter
{
    float4 Sample(Texture2D tex, int3 coord)
    {
        GMKernel kernel = {
            1.0 / 16, 2.0 / 16, 1.0 / 16,
            2.0 / 16, 4.0 / 16, 2.0 / 16,
            1.0 / 16, 2.0 / 16, 1.0 / 16  
        };
        return Kernel(kernel, tex, coord);
    }

    float4 SampleMSAA(Texture2DMS<float4> tex, int3 coord, int samplerId)
    {
        GMKernel kernel = {
            1.0 / 16, 2.0 / 16, 1.0 / 16,
            2.0 / 16, 4.0 / 16, 2.0 / 16,
            1.0 / 16, 2.0 / 16, 1.0 / 16  
        };
        return Kernel(kernel, tex, coord, samplerId);
    }
};

class GMGrayscaleFilter : IFilter
{
    float4 Sample(Texture2D tex, int3 coord)
    {
        float3 fragColor = tex.Load(coord).rgb;
        float average = 0.2126 * fragColor.r + 0.7152 * fragColor.g + 0.0722 * fragColor.b;
        return float4(average, average, average, 1);
    }

    float4 SampleMSAA(Texture2DMS<float4> tex, int3 coord, int samplerId)
    {
        float3 fragColor = tex.Load(coord, samplerId).rgb;
        float average = 0.2126 * fragColor.r + 0.7152 * fragColor.g + 0.0722 * fragColor.b;
        return float4(average, average, average, 1);
    }
};

class GMEdgeDetectFilter : IFilter
{
    float4 Sample(Texture2D tex, int3 coord)
    {
        GMKernel kernel = {
            1, 1, 1,
            1, -8, 1,
            1, 1, 1
        };
        return Kernel(kernel, tex, coord);
    }

    float4 SampleMSAA(Texture2DMS<float4> tex, int3 coord, int samplerId)
    {
        GMKernel kernel = {
            1, 1, 1,
            1, -8, 1,
            1, 1, 1
        };
        return Kernel(kernel, tex, coord, samplerId);
    }
};

GMDefaultFilter DefaultFilter;
GMInversionFilter InversionFilter;
GMSharpenFilter SharpenFilter;
GMBlurFilter BlurFilter;
GMGrayscaleFilter GrayscaleFilter;
GMEdgeDetectFilter EdgeDetectFilter;

IFilter Filter;
Texture2D FilterTexture;
Texture2DMS<float4> FilterTexture_MSAA;

VS_OUTPUT VS_Filter(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = float4(input.Position.x, input.Position.y, input.Position.z, 1);
    output.Texcoord = input.Texcoord;
    return output;
}

float4 PS_Filter(PS_INPUT input) : SV_TARGET
{
    int3 coord = int3(input.Texcoord * float2(ScreenInfo.ScreenWidth, ScreenInfo.ScreenHeight), 0);
    if (ScreenInfo.Multisampling)
        return Filter.SampleMSAA(FilterTexture_MSAA, coord, 0);

    return Filter.Sample(FilterTexture, coord);
}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 GMTech_3D
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_3D()));
        SetPixelShader(CompileShader(ps_5_0,PS_3D()));
        SetRasterizerState(GMRasterizerState);
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(GMDepthStencilState, 1);
    }
}

technique11 GMTech_2D
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_2D()));
        SetPixelShader(CompileShader(ps_5_0,PS_2D()));
        SetRasterizerState(GMRasterizerState);
        SetDepthStencilState(GMDepthStencilState, 1);
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_Glyph
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_Glyph()));
        SetPixelShader(CompileShader(ps_5_0,PS_Glyph()));
        SetRasterizerState(GMRasterizerState);
        SetDepthStencilState(GMDepthStencilState, 1);
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_CubeMap
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_CubeMap()));
        SetPixelShader(CompileShader(ps_5_0,PS_CubeMap()));
        SetRasterizerState(GMRasterizerState);
        SetDepthStencilState(GMDepthStencilState, 1);
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_Filter
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_Filter()));
        SetPixelShader(CompileShader(ps_5_0,PS_Filter()));
        SetRasterizerState(GMRasterizerState);
        SetDepthStencilState(GMDepthStencilState, 1);
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_Deferred_3D
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_3D_GeometryPass()));
        SetPixelShader(CompileShader(ps_5_0,PS_3D_GeometryPass()));
        SetRasterizerState(GMRasterizerState);
        SetDepthStencilState(GMDepthStencilState, 1);
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_Deferred_3D_LightPass
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_3D_LightPass()));
        SetPixelShader(CompileShader(ps_5_0,PS_3D_LightPass()));
        SetRasterizerState(GMRasterizerState);
        SetDepthStencilState(GMDepthStencilState, 1);
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_3D_Shadow
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_Shadow()));
        SetRasterizerState(GMRasterizerState);
        SetDepthStencilState(GMDepthStencilState, 1);
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}