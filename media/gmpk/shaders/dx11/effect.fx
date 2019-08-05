//--------------------------------------------------------------------------------------
// Functions
//--------------------------------------------------------------------------------------
static const float PI = 3.1415927f;

float4 GM_ToFloat4(float3 v, float w)
{
    return float4(v.x, v.y, v.z, w);
}

float4 GM_ToFloat4(float3 v)
{
    return GM_ToFloat4(v, 1);
}

float3x3 GM_ToFloat3x3(float4x4 m)
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
    matrix GM_WorldMatrix;
    matrix GM_ViewMatrix;
    matrix GM_ProjectionMatrix;
    matrix GM_InverseTransposeModelMatrix;
    matrix GM_InverseViewMatrix;
    float4 GM_ViewPosition;
}
//--------------------------------------------------------------------------------------
// Debug Options
//--------------------------------------------------------------------------------------
int GM_Debug_Normal = 0;
static const int GM_Debug_Normal_Off = 0;
static const int GM_Debug_Normal_WorldSpace = 1;
static const int GM_Debug_Normal_EyeSpace = 2;

//--------------------------------------------------------------------------------------
// Color Op
//--------------------------------------------------------------------------------------
int GM_ColorVertexOp = 0;
static const int GM_VertexColorOp_NoColor = 0;
static const int GM_VertexColorOp_Replace = 1;
static const int GM_VertexColorOp_Multiply = 2;
static const int GM_VertexColorOp_Add = 3;

//--------------------------------------------------------------------------------------
// Bones And Animations
//--------------------------------------------------------------------------------------
static const int GM_MaxBones = 128;
static const int GM_NoAnimation = 0;
static const int GM_SkeletalAnimation = 1;
static const int GM_AffineAnimation = 2;
int GM_UseAnimation = GM_NoAnimation;
matrix GM_Bones[GM_MaxBones];

//--------------------------------------------------------------------------------------
// Textures, GM_LightAttributes, Materials
//--------------------------------------------------------------------------------------
Texture2D GM_AmbientTexture;
Texture2D GM_DiffuseTexture;
Texture2D GM_SpecularTexture;
Texture2D GM_NormalMapTexture;
Texture2D GM_LightmapTexture;
Texture2D GM_AlbedoTexture;
Texture2D GM_MetallicRoughnessAOTexture;
TextureCube GM_CubeMapTexture;

SamplerState GM_AmbientSampler;
SamplerState GM_DiffuseSampler;
SamplerState GM_SpecularSampler;
SamplerState GM_NormalMapSampler;
SamplerState GM_LightmapSampler;
SamplerState GM_AlbedoSampler;
SamplerState GM_MetallicRoughnessAOSampler;
SamplerState GM_CubeMapSampler;

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
    return tex.Load(coord.xy, 0).xyz * 2.0f - 1.0f;
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

    float3 F0;
};
GMMaterial GM_Material;

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
        float2 transformedTexcoord = texcoord * float2(ScaleX, ScaleY) + float2(OffsetX, OffsetY);
        return TextureRGBToNormal(tex, ss, transformedTexcoord);
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

GMTexture GM_AmbientTextureAttribute;
GMTexture GM_DiffuseTextureAttribute;
GMTexture GM_SpecularTextureAttribute;
GMTexture GM_NormalMapTextureAttribute;
GMTexture GM_LightmapTextureAttribute;
GMTexture GM_AlbedoTextureAttribute;
GMTexture GM_MetallicRoughnessAOTextureAttribute;
GMCubeMapTexture GM_CubeMapTextureAttribute;

struct GM_Attenuation
{
    float Constant;
    float Linear;
    float Exp;
};

struct GMLight
{
    float4 Position;
    float4 Color;
    float4 AmbientIntensity;
    float4 DiffuseIntensity;
    float SpecularIntensity;
    int Type;
    GM_Attenuation Attenuation;

    // Directional
    float4 Direction;

    // Spotlight
    float CutOff;
};
GMLight GM_LightAttributes[50];

//--------------------------------------------------------------------------------------
// Phong 光照模型
//--------------------------------------------------------------------------------------
interface ILight
{
    float4 IlluminateAmbient();
    float4 IlluminateDiffuse(float3 lightDirection_N, float3 normal_N);
    float4 IlluminateSpecular(float3 lightDirection_N, float3 eyeDirection_N, float3 normal_N, float shininess);
};

float3 GetLightPosition_eye(float4 pos, float4x4 viewMatrix)
{
    return (mul(pos, viewMatrix)).xyz;
}

//默认的光照实现
class GMPhongPointLight : ILight
{
    float4 IlluminateAmbient()
    {
        return 1;
    }

    float4 IlluminateDiffuse(float3 lightDirection_N, float3 normal_N)
    {
        return max(dot(lightDirection_N, normal_N), 0);
    }

    float4 IlluminateSpecular(float3 lightDirection_N, float3 eyeDirection_N, float3 normal_N, float shininess)
    {
        float3 reflection_N = normalize(reflect(-lightDirection_N, normal_N));
        float theta = dot(eyeDirection_N, reflection_N);
        theta = max(theta, 0);
        float factor_Specular = (theta == 0 && shininess == 0) ? 0 : pow(max(theta, 0), shininess);
        return factor_Specular;
    }
};

GMPhongPointLight GM_PhongPointLight;

class GMPhongSpotlight
{
    float3 IlluminateSpotlightFactor(GMLight light, float3 lightDirection_World_N)
    {
        float spotFactor = dot(lightDirection_World_N, light.Direction.xyz); //light.Direction is world direction, we can use dot here
        if (spotFactor > light.CutOff)
        {
            float f = (1.0f - (1.0f - spotFactor) / (1.0f - light.CutOff));
            return float3(f, f, f);
        }
        else
        {
            return float3(0, 0, 0);
        }
    }
};
GMPhongSpotlight GM_PhongSpotlight;

static const int GM_PointLight = 0;
static const int GM_DirectionalLight = 1;
static const int GM_Spotlight = 2;

class GMPhongLightProxy
{
    float4 IlluminateAmbient(GMLight light)
    {
        if (light.Type == GM_PointLight || light.Type == GM_DirectionalLight)
            return GM_PhongPointLight.IlluminateAmbient();

        return GM_PhongPointLight.IlluminateAmbient();
    }

    float4 IlluminateDiffuse(GMLight light, float3 lightDirection_N, float3 normal_N)
    {
        if (light.Type == GM_PointLight || light.Type == GM_DirectionalLight)
            return GM_PhongPointLight.IlluminateDiffuse(lightDirection_N, normal_N);

        return GM_PhongPointLight.IlluminateDiffuse(lightDirection_N, normal_N);
    }

    float4 IlluminateSpecular(GMLight light, float3 lightDirection_N, float3 eyeDirection_N, float3 normal_N, float shininess)
    {
        if (light.Type == GM_PointLight || light.Type == GM_DirectionalLight)
            return GM_PhongPointLight.IlluminateSpecular(lightDirection_N, eyeDirection_N, normal_N, shininess);

        return GM_PhongPointLight.IlluminateSpecular(lightDirection_N, eyeDirection_N, normal_N, shininess);
    }

    float3 GetLightDirection_eye_N(GMLight light, float3 position_Eye)
    {
        if (light.Type == GM_PointLight || light.Type == GM_Spotlight)
        {
            return normalize(GetLightPosition_eye(light.Position, GM_ViewMatrix) - position_Eye);
        }
        else if (light.Type == GM_DirectionalLight)
        {
            return mul(-light.Direction.xyz, GM_ToFloat3x3(GM_ViewMatrix));
        }
        return float3(0,0,0);
    }

    float3 LightFactor(GMLight light, float3 lightDirection_World_N)
    {
        // 计算聚光灯下的因子
        if (light.Type == GM_Spotlight)
        {
            return GM_PhongSpotlight.IlluminateSpotlightFactor(light, lightDirection_World_N);
        }
        return float3(1, 1, 1);
    }
};
int GM_LightCount;

struct GMScreenInfo
{
    int ScreenWidth;
    int ScreenHeight;
    bool Multisampling;
};

GMScreenInfo GM_ScreenInfo;

//--------------------------------------------------------------------------------------
// Gamma Correction
//--------------------------------------------------------------------------------------
bool GM_GammaCorrection;
float GM_Gamma;
float GM_GammaInv;

float4 CalculateGammaCorrection(float4 factor)
{
    return GM_ToFloat4(pow(factor.rgb, float3(GM_GammaInv, GM_GammaInv, GM_GammaInv)));
}

float4 CalculateGammaCorrectionIfNecessary(float4 factor)
{
    if (!GM_GammaCorrection)
        return factor;

    return CalculateGammaCorrection(factor);
}

//--------------------------------------------------------------------------------------
// HDR
//--------------------------------------------------------------------------------------
bool GM_HDR;

interface IToneMapping
{
    float3 ToneMapping(float3 c);
};

class GMReinhardToneMapping : IToneMapping
{
    float3 ToneMapping(float3 c)
    {
        return c / (c + float3(1, 1, 1));
    }
};

IToneMapping GM_ToneMapping;
GMReinhardToneMapping ReinhardToneMapping;

float4 CalculateWithToneMapping(float4 c)
{
    if (GM_HDR)
        return GM_ToFloat4(GM_ToneMapping.ToneMapping(c.rgb));
    return c;
}

//--------------------------------------------------------------------------------------
// Shadow
//--------------------------------------------------------------------------------------
static const int GM_MaxCascadeLevel = 8;
struct GMShadowInfo
{
    bool HasShadow;
    matrix ShadowMatrix[GM_MaxCascadeLevel];
    float EndClip[GM_MaxCascadeLevel];
    int CurrentCascadeLevel;
    float4 Position;
    int ShadowMapWidth;
    int ShadowMapHeight;
    float BiasMin;
    float BiasMax;
    int CascadedShadowLevel;
    bool ViewCascade;
    int PCFRows;
};
Texture2D GM_ShadowMap;
Texture2DMS<float4> GM_ShadowMapMSAA;

GMShadowInfo GM_ShadowInfo;

// Cascade Color
static const float4 GM_CascadeColors[GM_MaxCascadeLevel] = {
    float4 ( 0.5f, 0.0f, 0.0f, 0.0f ),
    float4 ( 0.0f, 0.5f, 0.0f, 0.0f ),
    float4 ( 0.0f, 0.0f, 0.5f, 0.0f ),
    float4 ( 0.5f, 0.0f, 0.5f, 0.0f ),
    float4 ( 0.5f, 0.5f, 0.0f, 0.0f ),
    float4 ( 0.0f, 0.5f, 0.5f, 0.0f ),
    float4 ( 0.5f, 0.5f, 0.5f, 0.0f ),
    float4 ( 0.5f, 0.5f, 0.75f, 0.0f )
};

//--------------------------------------------------------------------------------------
// States
//--------------------------------------------------------------------------------------
RasterizerState GM_RasterizerState {};
BlendState GM_BlendState {};
DepthStencilState GM_DepthStencilState {};
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
    int4   BoneIDs     : BONES;
    float4 Weights     : WEIGHTS;
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
    float4 Z           : Z;
};

typedef VS_OUTPUT PS_INPUT;
typedef VS_OUTPUT GS_OUTPUT;

class GMTangentSpace
{
    float3 Normal_Tangent_N;
    float3x3 TBN; //TBN 表示从世界坐标向切线坐标转换的矩阵

    void CalculateTangentSpaceInEyeSpace(
        float2 texcoord,
        float3 tangent,
        float3 bitangent,
        float3 normal_Eye_N,
        float3x3 transform_Normal_Eye,
        GMTexture normalMap
        )
    {
        Normal_Tangent_N = normalMap.RGBToNormal(GM_NormalMapTexture, GM_NormalMapSampler, texcoord);
        float3 tangent_Eye_N = normalize(mul(tangent, transform_Normal_Eye).xyz);
        float3 bitangent_Eye_N = normalize(mul(bitangent, transform_Normal_Eye).xyz);
        TBN = transpose(float3x3(
            tangent_Eye_N,
            bitangent_Eye_N,
            normal_Eye_N
        ));
    }

    void CalculateTangentSpaceRuntime(
        float3 worldPos,
        float2 texcoord,
        float3 normal_World_N,
        GMTexture normalMap
        )
    {
        Normal_Tangent_N = normalMap.RGBToNormal(GM_NormalMapTexture, GM_NormalMapSampler, texcoord);

        float3 Q1 = ddx(worldPos);
        float3 Q2 = ddy(worldPos);
        float2 st1 = ddx(texcoord);
        float2 st2 = ddy(texcoord);

        float3 N = normalize(normal_World_N);
        float3 T = normalize(Q1 * st2.y - Q2 * st1.y);
        float3 B = -normalize(cross(N, T));
        TBN = transpose(float3x3(
            T,
            B,
            N
        ));
    }
};

float4 GM_IlluminateRefractionByNormalWorldN(
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
    return cubeMap.Sample(tex, GM_CubeMapSampler, refraction.xyz);
}

float4 GM_IlluminateRefraction(
    GMCubeMapTexture cubeMap,
    TextureCube tex,
    float3 normal_World_N,
    float4 position_World,
    float4 viewPosition_World,
    bool hasNormalMap,
    GMTangentSpace tangentSpace,
    float refractivity
    )
{
    if (refractivity == 0)
        return float4(0, 0, 0, 0);

    if (hasNormalMap)
    {
        // 如果是切线空间，计算会复杂点，要将切线空间的法线换算回世界空间
        float3 normal_Eye_N = normalize(mul(tangentSpace.Normal_Tangent_N, transpose(tangentSpace.TBN)));
        float3 normalFromTangent_World_N = mul(normal_Eye_N, GM_ToFloat3x3(GM_InverseViewMatrix));
        return GM_IlluminateRefractionByNormalWorldN(cubeMap, tex, normalFromTangent_World_N, position_World, viewPosition_World, refractivity);
    }

    return GM_IlluminateRefractionByNormalWorldN(cubeMap, tex, normal_World_N, position_World, viewPosition_World, refractivity);
}

//--------------------------------------------------------------------------------------
// 3D
//--------------------------------------------------------------------------------------

struct PS_3D_INPUT
{
    float Z;                   // 投影到世界后的Z坐标（未除以w）
    float3 WorldPos;            // 世界坐标
    float3 Normal_World_N;      // 世界法线
    float3 Normal_Eye_N;        // 眼睛空间法向量
    GMTangentSpace TangentSpace;  // 切线空间
    bool HasNormalMap;          // 是否有法线贴图
    float3 AmbientLightmapTexture;
    float3 DiffuseTexture;
    float3 SpecularTexture;
    float Shininess;
    float Refractivity;
    float3 AlbedoTexture;
    float3 MetallicRoughnessAOTexture;
    float3 F0;
    int IlluminationModel;
};

interface IIlluminationModel
{
    float4 Calculate(PS_3D_INPUT input, float shadowFactor);
};
int GM_IlluminationModel = 0;


float GM_CalculateShadow_Multisampling(float3 projCoords, int cascade, float bias)
{
    float result = 0;
    float closestDepth = 0;

    // 每一份Shadow Map的宽度
    int pieceWidth = (uint) GM_ShadowInfo.ShadowMapWidth / (uint) GM_ShadowInfo.CascadedShadowLevel;

    int x = pieceWidth * (projCoords.x + cascade);
    int y = GM_ShadowInfo.ShadowMapHeight * projCoords.y;
    int samples = 0;
    int bound = GM_ShadowInfo.PCFRows - 1;
    for (int i = -bound; i <= bound; ++i)
    {
        for (int j = -bound; j <= bound; ++j)
        {
            if ((x + i >= 0 && y + j >= 0) && (x + i <= GM_ShadowInfo.ShadowMapWidth && y + j <= GM_ShadowInfo.ShadowMapHeight) )
            {
                closestDepth = GM_ShadowMapMSAA.Load(int2(x + i, y + j), 0).x;
                result += (projCoords.z - bias) > closestDepth ? 0.f : 1.f;
                ++samples;
            }
        }
    }
    result /= samples;
    return result;
}

float GM_CalculateShadow(PS_3D_INPUT input)
{
    if (!GM_ShadowInfo.HasShadow)
        return 1.0f;

    int cascade = 0;
    if (GM_ShadowInfo.CascadedShadowLevel > 1)
    {
        for (int i = 0; i < GM_ShadowInfo.CascadedShadowLevel; ++i)
        {
            if (input.Z <= GM_ShadowInfo.EndClip[i])
            {
                cascade = i;
                break;
            }
        }
    }

    float4 worldPos = GM_ToFloat4(input.WorldPos);
    float3 normal_N = input.Normal_World_N;
    float4 fragPos = mul(worldPos, GM_ShadowInfo.ShadowMatrix[cascade]);
    float3 projCoords = fragPos.xyz / fragPos.w;
    if (projCoords.z > 1.0f)
        return 1.0f;

    projCoords.x = projCoords.x * 0.5f + 0.5f;
    projCoords.y = projCoords.y * (-0.5f) + 0.5f;

    if (projCoords.x > 1 || projCoords.x < 0 ||
    projCoords.y > 1 || projCoords.y < 0 )
    {
        return 1.f;
    }
    
    float bias = (GM_ShadowInfo.BiasMin == GM_ShadowInfo.BiasMax) ? GM_ShadowInfo.BiasMin : max(GM_ShadowInfo.BiasMax * (1.0 - dot(normal_N, normalize(worldPos.xyz - GM_ShadowInfo.Position.xyz))), GM_ShadowInfo.BiasMin);
    if (GM_ScreenInfo.Multisampling)
        return GM_CalculateShadow_Multisampling(projCoords, cascade, bias);

    // 非多重采样不支持PCF
    float closestDepth = 0;
    if (GM_ShadowInfo.CascadedShadowLevel == 1)
    {
        closestDepth = GM_ShadowMap.Sample(ShadowMapSampler, projCoords.xy).r;
    }
    else
    {
        // 每一份Shadow Map的缩放。例如，假设Cascade Level = 3，那么第一幅Shadow Map采样范围就是0~0.333。
        float projRatio = 1.f / GM_ShadowInfo.CascadedShadowLevel;

        float2 projCoordsInCSM = float2(projRatio * (projCoords.x + cascade), projCoords.y);
        closestDepth = GM_ShadowMap.Sample(ShadowMapSampler, projCoordsInCSM.xy).r;
    }

    return projCoords.z - bias > closestDepth ? 0.f : 1.f;
}

float4 GM_ViewCascade(PS_3D_INPUT input)
{
    float4 cascadeIndicator = float4(0, 0, 0, 0);
    if (GM_ShadowInfo.HasShadow && GM_ShadowInfo.ViewCascade)
    {
        for (int i = 0; i < GM_ShadowInfo.CascadedShadowLevel; ++i)
        {
            if (input.Z <= GM_ShadowInfo.EndClip[i])
            {
                cascadeIndicator = GM_CascadeColors[i];
                break;
            }
        }
    }
    return cascadeIndicator;
}

class GMPhong : IIlluminationModel
{
    float4 Calculate(PS_3D_INPUT input, float shadowFactor)
    {
        float4 factor_Ambient = float4(0, 0, 0, 0);
        float4 factor_Diffuse = float4(0, 0, 0, 0);
        float4 factor_Specular = float4(0, 0, 0, 0);
        input.AmbientLightmapTexture = max(input.AmbientLightmapTexture, float3(0, 0, 0));
        input.DiffuseTexture = max(input.DiffuseTexture, float3(0, 0, 0));
        input.SpecularTexture = max(input.SpecularTexture, float3(0, 0, 0));

        // 将法线换算到眼睛坐标系
        float3 position_Eye = (mul(GM_ToFloat4(input.WorldPos), GM_ViewMatrix)).xyz;
        float3 position_Eye_N = normalize(position_Eye);

        for (int i = 0; i < GM_LightCount; ++i)
        {
            float distance = length(input.WorldPos - GM_LightAttributes[i].Position.xyz);
            float attenuation = GM_LightAttributes[i].Attenuation.Constant + 
                                GM_LightAttributes[i].Attenuation.Linear * distance +
                                GM_LightAttributes[i].Attenuation.Exp * distance * distance;
            float3 lightDirection_World_N = normalize(input.WorldPos - GM_LightAttributes[i].Position.xyz);
            float spotFactor = PhoneLightProxy.LightFactor(GM_LightAttributes[i], lightDirection_World_N).x;

            factor_Ambient += spotFactor * PhoneLightProxy.IlluminateAmbient(GM_LightAttributes[i]) * GM_LightAttributes[i].Color * GM_LightAttributes[i].AmbientIntensity / attenuation;

            if (!input.HasNormalMap)
            {
                float3 lightDirection_Eye_N = PhoneLightProxy.GetLightDirection_eye_N(GM_LightAttributes[i], position_Eye);
                factor_Diffuse += spotFactor * PhoneLightProxy.IlluminateDiffuse(GM_LightAttributes[i], lightDirection_Eye_N, input.Normal_Eye_N) * GM_LightAttributes[i].Color * GM_LightAttributes[i].DiffuseIntensity / attenuation;
                factor_Specular += spotFactor * PhoneLightProxy.IlluminateSpecular(GM_LightAttributes[i], lightDirection_Eye_N, -position_Eye_N, input.Normal_Eye_N, input.Shininess) * GM_LightAttributes[i].Color * GM_LightAttributes[i].SpecularIntensity / attenuation;
            }
            else
            {
                float3 lightDirection_Eye_N = PhoneLightProxy.GetLightDirection_eye_N(GM_LightAttributes[i], position_Eye);
                float3 lightDirection_Tangent_N = normalize(mul(lightDirection_Eye_N, input.TangentSpace.TBN));
                float3 eyeDirection_Tangent_N = normalize(mul(-position_Eye, input.TangentSpace.TBN));
                factor_Diffuse += spotFactor * PhoneLightProxy.IlluminateDiffuse(GM_LightAttributes[i], lightDirection_Tangent_N, input.TangentSpace.Normal_Tangent_N) * GM_LightAttributes[i].Color * GM_LightAttributes[i].DiffuseIntensity / attenuation;
                factor_Specular += spotFactor * PhoneLightProxy.IlluminateSpecular(GM_LightAttributes[i], lightDirection_Tangent_N, eyeDirection_Tangent_N, input.TangentSpace.Normal_Tangent_N, input.Shininess) * GM_LightAttributes[i].Color * GM_LightAttributes[i].SpecularIntensity / attenuation; 
            }
        }
        float4 color_Ambient = CalculateGammaCorrectionIfNecessary(factor_Ambient) * GM_ToFloat4(input.AmbientLightmapTexture);
        float4 color_Diffuse = shadowFactor * CalculateGammaCorrectionIfNecessary(factor_Diffuse) * GM_ToFloat4(input.DiffuseTexture);

        // 计算Specular
        float4 color_Specular = shadowFactor * CalculateGammaCorrectionIfNecessary(factor_Specular) * GM_ToFloat4(input.SpecularTexture);
        
        // 计算折射
        float4 color_Refractivity = GM_IlluminateRefraction(
            GM_CubeMapTextureAttribute,
            GM_CubeMapTexture,
            input.Normal_World_N,
            GM_ToFloat4(input.WorldPos),
            GM_ViewPosition,
            input.HasNormalMap,
            input.TangentSpace,
            input.Refractivity
            );

        return color_Ambient + color_Diffuse + color_Specular + color_Refractivity;
    }

    GMPhongLightProxy PhoneLightProxy;
};
GMPhong GM_Phong;

class GMCookTorranceBRDF : IIlluminationModel
{
    float DistributionGGX(float3 N, float3 H, float roughness)
    {
        float a = roughness*roughness;
        float a2 = a*a;
        float NdotH = max(dot(N, H), 0.0);
        float NdotH2 = NdotH*NdotH;

        float nom   = a2;
        float denom = (NdotH2 * (a2 - 1.0) + 1.0);
        denom = PI * denom * denom;

        return nom / denom;
    }
    // ----------------------------------------------------------------------------
    float GeometrySchlickGGX(float NdotV, float roughness)
    {
        float r = (roughness + 1.0);
        float k = (r*r) / 8.0;

        float nom   = NdotV;
        float denom = NdotV * (1.0 - k) + k;

        return nom / denom;
    }
    // ----------------------------------------------------------------------------
    float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
    {
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);
        float ggx2 = GeometrySchlickGGX(NdotV, roughness);
        float ggx1 = GeometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
    }
    // ----------------------------------------------------------------------------
    float3 FresnelSchlick(float cosTheta, float3 F0)
    {
        return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    }

    float4 Calculate(PS_3D_INPUT input, float shadowFactor)
    {
        float3 viewDirection_N = normalize(GM_ViewPosition.rgb - input.WorldPos);

        // 换算回世界空间
        float3 normal_World_N = normalize(mul(input.TangentSpace.Normal_Tangent_N, transpose(input.TangentSpace.TBN)));
        float metallic = input.MetallicRoughnessAOTexture.r;
        float roughness = input.MetallicRoughnessAOTexture.g;
        float ao = input.MetallicRoughnessAOTexture.b;
        float3 F0 = lerp(input.F0, input.AlbedoTexture, metallic);
        float3 Lo = float3(0, 0, 0);
        float3 ambient = float3(0, 0, 0);

        for (int i = 0; i < GM_LightCount; ++i)
        {
            // 只考虑直接光源
            ambient += GM_LightAttributes[i].Color.rgb * GM_LightAttributes[i].AmbientIntensity.xyz * input.AlbedoTexture * roughness;
            // 计算每束光辐射率
            float3 L_N = normalize(GM_LightAttributes[i].Position.xyz - input.WorldPos);
            float3 H_N = normalize(viewDirection_N + L_N);

            float distance = length(input.WorldPos - GM_LightAttributes[i].Position.xyz);
            float attenuation = GM_LightAttributes[i].Attenuation.Constant + 
                                GM_LightAttributes[i].Attenuation.Linear * distance +
                                GM_LightAttributes[i].Attenuation.Exp * distance * distance;

            float3 radiance = GM_LightAttributes[i].Color.rgb * GM_LightAttributes[i].DiffuseIntensity.xyz * attenuation;

            // Cook-Torrance BRDF
            float NDF = DistributionGGX(normal_World_N, H_N, roughness);
            float G = GeometrySmith(normal_World_N, viewDirection_N, L_N, roughness);
            float3 F = FresnelSchlick(max(dot(H_N, viewDirection_N), 0.0f), F0);
            float3 nominator = NDF * G * F;
            float denominator = 4 * max(dot(normal_World_N, viewDirection_N), 0.0) * max(dot(normal_World_N, L_N), 0.0) + 0.001; // 0.001 防止除0
            float3 specular = nominator / denominator;

            float3 ks = F;
            float3 kd = float3(1, 1, 1) - ks;
            kd *= 1.0f - metallic;

            float cosTheta = max(dot(normal_World_N, L_N), 0);
            Lo += (kd * input.AlbedoTexture / PI + specular) * radiance * cosTheta;
        }

        float4 color = GM_ToFloat4(ambient + Lo) * shadowFactor;
        color.rgb = ReinhardToneMapping.ToneMapping(color.rgb);
        color = CalculateGammaCorrection(color);
        return color;
    }
};
GMCookTorranceBRDF GM_CookTorranceBRDF;

static const int GM_IlluminationModel_None = 0;
static const int GM_IlluminationModel_Phong = 1;
static const int GM_IlluminationModel_CookTorranceBRDF = 2;

float4 PS_3D_CalculateColor(PS_3D_INPUT input)
{
    float factor_Shadow = GM_CalculateShadow(input);
    float4 csmIndicator = GM_ViewCascade(input);
    switch (input.IlluminationModel)
    {
        case GM_IlluminationModel_None:
            discard;
            break;
        case GM_IlluminationModel_Phong:
            return csmIndicator + GM_Phong.Calculate(input, factor_Shadow);
        case GM_IlluminationModel_CookTorranceBRDF:
            return csmIndicator + GM_CookTorranceBRDF.Calculate(input, factor_Shadow);
    }
    return float4(0, 0, 0, 0);
}

VS_OUTPUT VS_3D( VS_INPUT input )
{
    VS_OUTPUT output;
    output.Position = GM_ToFloat4(input.Position);

    if (GM_UseAnimation == GM_SkeletalAnimation)
    {
        matrix boneTransform = GM_Bones[input.BoneIDs[0]] * input.Weights[0];
        boneTransform += GM_Bones[input.BoneIDs[1]] * input.Weights[1];
        boneTransform += GM_Bones[input.BoneIDs[2]] * input.Weights[2];
        boneTransform += GM_Bones[input.BoneIDs[3]] * input.Weights[3];
        output.Position = mul(output.Position, boneTransform);
        output.Normal = mul(input.Normal, GM_ToFloat3x3(boneTransform));
    }
    else if (GM_UseAnimation == GM_AffineAnimation)
    {
        // 将仿射变换的值放到第1个Bones矩阵中
        output.Position = mul(output.Position, GM_Bones[0]);
        output.Normal = mul(GM_ToFloat4(input.Normal.xyz), GM_Bones[0]);
    }
    else
    {
        output.Normal = input.Normal;
    }

    output.Position = mul(output.Position, GM_WorldMatrix);
    output.WorldPos = output.Position;
    
    output.Position = mul(output.Position, GM_ViewMatrix);
    output.Position = mul(output.Position, GM_ProjectionMatrix);

    output.Texcoord = input.Texcoord;
    output.Tangent = input.Tangent;
    output.Bitangent = input.Bitangent;
    output.Lightmap = input.Lightmap;
    output.Color = input.Color;
    output.Z = output.Position.z;
    return output;
}

bool PS_3D_HasNormalMap()
{
    return GM_NormalMapTextureAttribute.Enabled;
}

bool GM_IsTangentSpaceInvalid(float3 tangent, float3 bitangent)
{
    // 返回是否无切线空间
    return length(tangent) < 0.01f && length(bitangent) < 0.01f;
}

float4 GM_NormalToTexture(float3 normal_N)
{
    // 先反转z，按照大家的习惯来展现法线颜色
    normal_N = float3(normal_N.xy, -normal_N.z);
    return float4( (normal_N + 1) * .5f, 1);
}

GMTexture PS_3D_NormalMap()
{
    return GM_NormalMapTextureAttribute;
}

float4 PS_3D(PS_INPUT input) : SV_TARGET
{
    if (GM_ColorVertexOp == GM_VertexColorOp_Replace)
    {
        return input.Color;
    }

    // 将法线换算到眼睛坐标系
    float3x3 inverseTransposeModelMatrix = GM_ToFloat3x3(GM_InverseTransposeModelMatrix);
    float3x3 transform_Normal_Eye = mul(inverseTransposeModelMatrix, GM_ToFloat3x3(GM_ViewMatrix));
    float3 normal_Eye_N = normalize(mul(input.Normal, transform_Normal_Eye));

    PS_3D_INPUT commonInput;
    commonInput.Z = input.Z.x;
    commonInput.Normal_World_N = normalize(mul(input.Normal, inverseTransposeModelMatrix));
    GMTangentSpace tangentSpace;
    if (PS_3D_HasNormalMap())
    {
        if (GM_IsTangentSpaceInvalid(input.Tangent, input.Bitangent))
            tangentSpace.CalculateTangentSpaceRuntime(input.WorldPos.xyz, input.Texcoord, commonInput.Normal_World_N, PS_3D_NormalMap());
        else
            tangentSpace.CalculateTangentSpaceInEyeSpace(input.Texcoord, input.Tangent, input.Bitangent, normal_Eye_N, transform_Normal_Eye, PS_3D_NormalMap());
    }

    commonInput.TangentSpace = tangentSpace;
    commonInput.WorldPos = input.WorldPos.xyz;
    commonInput.HasNormalMap = PS_3D_HasNormalMap();

    commonInput.Normal_Eye_N = normal_Eye_N;
    commonInput.IlluminationModel = GM_IlluminationModel;

    /// Start Debug Option
    if (GM_Debug_Normal == GM_Debug_Normal_WorldSpace)
    {
        if (commonInput.HasNormalMap)
            return GM_NormalToTexture(normalize(mul(mul(tangentSpace.Normal_Tangent_N, transpose(tangentSpace.TBN)), GM_ToFloat3x3(GM_InverseViewMatrix))));
        else
            return GM_NormalToTexture(commonInput.Normal_World_N.xyz);
    }
    else if (GM_Debug_Normal == GM_Debug_Normal_EyeSpace)
    {
        if (commonInput.HasNormalMap)
            return GM_NormalToTexture(normalize(mul(tangentSpace.Normal_Tangent_N, transpose(tangentSpace.TBN))));
        else
            return GM_NormalToTexture(commonInput.Normal_Eye_N.xyz);
    }
    /// End Debug Option

    // 计算Ambient
    float4 color_Ambient = GM_AmbientTextureAttribute.Sample(GM_AmbientTexture, GM_AmbientSampler, input.Texcoord);
    color_Ambient *= GM_LightmapTextureAttribute.Sample(GM_LightmapTexture, GM_LightmapSampler, input.Lightmap);

    // 计算Diffuse
    float4 color_Diffuse = GM_DiffuseTextureAttribute.Sample(GM_DiffuseTexture, GM_DiffuseSampler, input.Texcoord);

    // 计算Specular(如果有Specular贴图)
    float4 color_Specular = GM_SpecularTextureAttribute.Sample(GM_SpecularTexture, GM_SpecularSampler, input.Texcoord).r;

    if (GM_IlluminationModel == GM_IlluminationModel_Phong)
    {
        commonInput.AmbientLightmapTexture = (color_Ambient * GM_Material.Ka).xyz;
        commonInput.DiffuseTexture = (color_Diffuse * GM_Material.Kd).xyz;
        commonInput.SpecularTexture = (color_Specular * GM_Material.Ks).xyz;
        commonInput.Shininess = GM_Material.Shininess;
        commonInput.Refractivity = GM_Material.Refractivity;
    }
    else if (GM_IlluminationModel == GM_IlluminationModel_CookTorranceBRDF)
    {
        commonInput.AlbedoTexture = pow( GM_AlbedoTextureAttribute.Sample(GM_AlbedoTexture, GM_AlbedoSampler, input.Texcoord).rgb, float3(GM_Gamma, GM_Gamma, GM_Gamma));
        commonInput.MetallicRoughnessAOTexture = GM_MetallicRoughnessAOTextureAttribute.Sample(GM_MetallicRoughnessAOTexture, GM_MetallicRoughnessAOSampler, input.Texcoord).rgb;
        commonInput.F0 = GM_Material.F0;
    }

    float4 fragColor = PS_3D_CalculateColor(commonInput);
    if (GM_ColorVertexOp == GM_VertexColorOp_Multiply)
        fragColor *= input.Color;
    else if (GM_ColorVertexOp == GM_VertexColorOp_Add)
        fragColor += input.Color;
    // else (GM_ColorVertexOp == 0) do nothing
    return fragColor;
}

//--------------------------------------------------------------------------------------
// 2D
//--------------------------------------------------------------------------------------
VS_OUTPUT VS_2D(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = float4(input.Position.x, input.Position.y, input.Position.z, 1);
    output.Position = mul(output.Position, GM_WorldMatrix);
    output.Normal = input.Normal;
    output.Texcoord = input.Texcoord;
    output.Tangent = input.Tangent;
    output.Bitangent = input.Bitangent;
    output.Lightmap = input.Lightmap;
    output.Color = input.Color;
    output.Z = output.Position.z;
    return output;
}

float4 PS_2D(PS_INPUT input) : SV_TARGET
{
    if (GM_ColorVertexOp == GM_VertexColorOp_Replace)
        return input.Color;

    if (!GM_DiffuseTextureAttribute.Enabled && !GM_AmbientTextureAttribute.Enabled)
        return float4(0, 0, 0, 0);

    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    if (GM_AmbientTextureAttribute.Enabled)
        color += GM_AmbientTextureAttribute.Sample(GM_AmbientTexture, GM_AmbientSampler, input.Texcoord);

    if (GM_DiffuseTextureAttribute.Enabled)
        color += GM_DiffuseTextureAttribute.Sample(GM_DiffuseTexture, GM_DiffuseSampler, input.Texcoord);

    if (GM_ColorVertexOp == GM_VertexColorOp_NoColor)
        return color;
    else if (GM_ColorVertexOp == GM_VertexColorOp_Add)
        return color + input.Color;

    return color * input.Color;
}

//--------------------------------------------------------------------------------------
// Glyph
//--------------------------------------------------------------------------------------
VS_OUTPUT VS_Text(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = GM_ToFloat4(input.Position);
    output.Position = mul(output.Position, GM_WorldMatrix);
    output.Position.z = 0;
    output.Position.w = 1;
    output.Normal = input.Normal;
    output.Texcoord = input.Texcoord;
    output.Tangent = input.Tangent;
    output.Bitangent = input.Bitangent;
    output.Lightmap = input.Lightmap;
    output.Color = input.Color;
    output.Z = output.Position.z;
    return output;
}

float4 PS_Text(PS_INPUT input) : SV_TARGET
{
    float4 alpha = GM_AmbientTexture.Sample(GM_AmbientSampler, input.Texcoord);
    return float4(input.Color.r, input.Color.g, input.Color.b, alpha.r * input.Color.a);
}

//--------------------------------------------------------------------------------------
// CubeMap
//--------------------------------------------------------------------------------------
VS_OUTPUT VS_CubeMap(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = float4(input.Position.x, input.Position.y, input.Position.z, 1);
    output.WorldPos = output.Position;
    output.Position = mul(output.Position, GM_WorldMatrix);
    output.Position = mul(output.Position, GM_ViewMatrix);
    output.Position = mul(output.Position, GM_ProjectionMatrix);
    output.Z = output.Position.z;
    return output;
}

float4 PS_CubeMap(PS_INPUT input) : SV_TARGET
{
    float3 texcoord = input.WorldPos.xyz;
    return GM_CubeMapTextureAttribute.Sample(GM_CubeMapTexture, GM_CubeMapSampler, texcoord);
}

//--------------------------------------------------------------------------------------
// Deferred 3D
//--------------------------------------------------------------------------------------
Texture2D GM_DeferredPosition_World_Refractivity;
Texture2D GM_DeferredNormal_World_IlluminationModel;
Texture2D GM_DeferredTextureAmbientAlbedo;
Texture2D GM_DeferredTextureDiffuseMetallicRoughnessAO;
Texture2D GM_DeferredTangent_Eye;
Texture2D GM_DeferredBitangent_Eye;
Texture2D GM_DeferredNormalMap_bNormalMap;
Texture2D GM_DeferredSpecular_Shininess_F0;

Texture2DMS<float4> GM_DeferredPosition_World_Refractivity_MSAA;
Texture2DMS<float4> GM_DeferredNormal_World_IlluminationModel_MSAA;
Texture2DMS<float4> GM_DeferredTextureAmbientAlbedo_MSAA;
Texture2DMS<float4> GM_DeferredTextureDiffuseMetallicRoughnessAO_MSAA;
Texture2DMS<float4> GM_DeferredTangent_Eye_MSAA;
Texture2DMS<float4> GM_DeferredBitangent_Eye_MSAA;
Texture2DMS<float4> GM_DeferredNormalMap_bNormalMap_MSAA;
Texture2DMS<float4> GM_DeferredSpecular_Shininess_F0_MSAA;

// [-1, 1] -> [0, 1]
float4 Float3ToTexture(float3 normal)
{
    return GM_ToFloat4((normal + 1) * .5f, 1);
}

struct VS_GEOMETRY_OUTPUT
{
    float4 Position_Refractivity             : SV_TARGET0;
    float4 Normal_World_IlluminationModel    : SV_TARGET1;
    float4 TextureAmbientAlbedo              : SV_TARGET2;
    float4 TextureDiffuseMetallicRoughnessAO : SV_TARGET3;
    float4 Tangent_Eye                       : SV_TARGET4; //需要将区间转化到[0, 1]
    float4 Bitangent_Eye                     : SV_TARGET5; //需要将区间转化到[0, 1]
    float4 NormalMap_HasNormalMap            : SV_TARGET6;
    float4 Specular_Shininess_F0             : SV_TARGET7;
};

VS_OUTPUT VS_3D_GeometryPass(VS_INPUT input)
{
    return VS_3D(input);
}

VS_GEOMETRY_OUTPUT PS_3D_GeometryPass(PS_INPUT input)
{
    if (GM_IlluminationModel == GM_IlluminationModel_None)
        discard;

    VS_GEOMETRY_OUTPUT output;
    output.Position_Refractivity.rgb = input.WorldPos.xyz;
    output.Position_Refractivity.a = GM_Material.Refractivity;

    float3x3 inverseTransposeModelMatrix = GM_ToFloat3x3(GM_InverseTransposeModelMatrix);
    float3x3 normalEyeTransform = mul(inverseTransposeModelMatrix, GM_ToFloat3x3(GM_ViewMatrix));
    float3 normal_World = normalize(mul(input.Normal.xyz, inverseTransposeModelMatrix));
    output.Normal_World_IlluminationModel = Float3ToTexture(normal_World);
    output.Normal_World_IlluminationModel.a = GM_IlluminationModel;

    if (GM_IlluminationModel == GM_IlluminationModel_Phong)
    {
        float4 texAmbient = GM_AmbientTextureAttribute.Sample(GM_AmbientTexture, GM_AmbientSampler, input.Texcoord);
        texAmbient *= GM_LightmapTextureAttribute.Sample(GM_LightmapTexture, GM_LightmapSampler, input.Lightmap);
        float4 texDiffuse = GM_DiffuseTextureAttribute.Sample(GM_DiffuseTexture, GM_DiffuseSampler, input.Texcoord);
        output.TextureAmbientAlbedo = texAmbient * GM_Material.Ka;
        output.TextureDiffuseMetallicRoughnessAO = texDiffuse * GM_Material.Kd;
        float texSpecular = GM_SpecularTextureAttribute.Sample(GM_SpecularTexture, GM_SpecularSampler, input.Texcoord).r;
        output.Specular_Shininess_F0 = GM_ToFloat4(GM_Material.Ks.rgb * texSpecular, GM_Material.Shininess);
    }
    else if (GM_IlluminationModel == GM_IlluminationModel_CookTorranceBRDF)
    {
        output.TextureAmbientAlbedo = GM_AlbedoTextureAttribute.Sample(GM_AlbedoTexture, GM_AlbedoSampler, input.Texcoord);
        output.TextureDiffuseMetallicRoughnessAO = GM_MetallicRoughnessAOTextureAttribute.Sample(GM_MetallicRoughnessAOTexture, GM_MetallicRoughnessAOSampler, input.Texcoord);
        output.Specular_Shininess_F0 = GM_ToFloat4(GM_Material.F0);
    }

    if (PS_3D_HasNormalMap())
    {
        if (GM_IsTangentSpaceInvalid(input.Tangent, input.Bitangent))
        {
            GMTangentSpace tangentSpace;
            tangentSpace.CalculateTangentSpaceRuntime(input.WorldPos.xyz, input.Texcoord, normal_World, PS_3D_NormalMap());
            float3x3 TBN = transpose(tangentSpace.TBN);
            output.Tangent_Eye = Float3ToTexture(normalize(TBN[0]));
            output.Bitangent_Eye = Float3ToTexture(normalize(TBN[1])); 
        }
        else
        {
            output.Tangent_Eye = Float3ToTexture(normalize(mul(input.Tangent, normalEyeTransform)));
            output.Bitangent_Eye = Float3ToTexture(normalize(mul(input.Bitangent, normalEyeTransform)));
        }
        output.NormalMap_HasNormalMap = GM_ToFloat4(PS_3D_NormalMap().Sample(GM_NormalMapTexture, GM_NormalMapSampler, input.Texcoord).xyz, 1);
    }
    else
    {
        output.Tangent_Eye = Float3ToTexture(float3(0, 0, 0));
        output.Bitangent_Eye = Float3ToTexture(float3(0, 0, 0));
        output.NormalMap_HasNormalMap = float4(0, 0, 0, 0);
    }

    return output;
}

VS_OUTPUT VS_3D_LightPass(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = GM_ToFloat4(input.Position);
    output.Texcoord = input.Texcoord;
    output.Z = output.Position.z;
    return output;
}

float4 PS_3D_LightPass(PS_INPUT input) : SV_TARGET
{
    PS_3D_INPUT commonInput;
    commonInput.Z = input.Z.x;

    int3 coord = int3(input.Texcoord * float2(GM_ScreenInfo.ScreenWidth, GM_ScreenInfo.ScreenHeight), 0);
    GMTangentSpace tangentSpace;
    float3 tangent_Eye_N;
    float3 bitangent_Eye_N;
    float4 PosRef;
    float4 KSS;
    float4 normalMapFlag;

    commonInput.AmbientLightmapTexture = 
    commonInput.DiffuseTexture =
    commonInput.SpecularTexture =
    commonInput.AlbedoTexture =
    commonInput.MetallicRoughnessAOTexture =
    commonInput.F0 = float3(0, 0, 0);
    commonInput.Shininess = commonInput.Refractivity = 0;

    if (!GM_ScreenInfo.Multisampling)
    {
        tangent_Eye_N = TextureRGBToNormal(GM_DeferredTangent_Eye, coord).rgb;
        bitangent_Eye_N = TextureRGBToNormal(GM_DeferredBitangent_Eye, coord).rgb;
        PosRef = GM_DeferredPosition_World_Refractivity.Load(coord);
        commonInput.Normal_World_N = TextureRGBToNormal(GM_DeferredNormal_World_IlluminationModel, coord);
        commonInput.IlluminationModel = (int)(GM_DeferredNormal_World_IlluminationModel.Load(coord).a);

        if (commonInput.IlluminationModel == GM_IlluminationModel_None)
        {
            discard;
        }
        else if (commonInput.IlluminationModel == GM_IlluminationModel_Phong)
        {
            commonInput.AmbientLightmapTexture = (GM_DeferredTextureAmbientAlbedo.Load(coord)).rgb;
            commonInput.DiffuseTexture = (GM_DeferredTextureDiffuseMetallicRoughnessAO.Load(coord)).rgb;
            KSS = GM_DeferredSpecular_Shininess_F0.Load(coord);
            commonInput.SpecularTexture = KSS.rgb;
            commonInput.Shininess = KSS.a;
            commonInput.Refractivity = PosRef.a;
        }
        else if (commonInput.IlluminationModel == GM_IlluminationModel_CookTorranceBRDF)
        {
            commonInput.AlbedoTexture = pow(GM_DeferredTextureAmbientAlbedo.Load(coord).rgb, float3(GM_Gamma, GM_Gamma, GM_Gamma));
            commonInput.MetallicRoughnessAOTexture = (GM_DeferredTextureDiffuseMetallicRoughnessAO.Load(coord)).rgb;
            commonInput.F0 = GM_DeferredSpecular_Shininess_F0.Load(coord).rgb;
        }

        normalMapFlag = GM_DeferredNormalMap_bNormalMap.Load(coord);
    }
    else
    {
        tangent_Eye_N = TextureRGBToNormal(GM_DeferredTangent_Eye_MSAA, coord).rgb;
        bitangent_Eye_N = TextureRGBToNormal(GM_DeferredBitangent_Eye_MSAA, coord).rgb;
        PosRef = GM_DeferredPosition_World_Refractivity_MSAA.Load(coord.xy, 0);
        commonInput.Normal_World_N = TextureRGBToNormal(GM_DeferredNormal_World_IlluminationModel_MSAA, coord);
        commonInput.IlluminationModel = (int)(GM_DeferredNormal_World_IlluminationModel_MSAA.Load(coord.xy, 0).a);

        if (commonInput.IlluminationModel == GM_IlluminationModel_Phong)
        {
            commonInput.AmbientLightmapTexture = (GM_DeferredTextureAmbientAlbedo_MSAA.Load(coord.xy, 0)).rgb;
            commonInput.DiffuseTexture = (GM_DeferredTextureDiffuseMetallicRoughnessAO_MSAA.Load(coord.xy, 0)).rgb;
            KSS = GM_DeferredSpecular_Shininess_F0_MSAA.Load(coord.xy, 0);
            commonInput.SpecularTexture = KSS.rgb;
            commonInput.Shininess = KSS.a;
            commonInput.Refractivity = PosRef.a;
        }
        else if (commonInput.IlluminationModel == GM_IlluminationModel_CookTorranceBRDF)
        {
            commonInput.AlbedoTexture = pow((GM_DeferredTextureAmbientAlbedo_MSAA.Load(coord.xy, 0)).rgb, float3(GM_Gamma, GM_Gamma, GM_Gamma));
            commonInput.MetallicRoughnessAOTexture = (GM_DeferredTextureDiffuseMetallicRoughnessAO_MSAA.Load(coord.xy, 0)).rgb;
            commonInput.F0 = GM_DeferredSpecular_Shininess_F0_MSAA.Load(coord.xy, 0).xyz;
        }

        normalMapFlag = GM_DeferredNormalMap_bNormalMap_MSAA.Load(coord.xy, 0);
    }
    commonInput.WorldPos = PosRef.rgb;
    commonInput.HasNormalMap = (normalMapFlag.a != 0);
    commonInput.Normal_Eye_N = mul(commonInput.Normal_World_N, GM_ToFloat3x3(GM_ViewMatrix));

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
// Particle
//--------------------------------------------------------------------------------------
VS_OUTPUT VS_Particle(VS_INPUT input)
{
    return VS_3D(input);
}

float4 PS_Particle(PS_INPUT input) : SV_TARGET
{
    float4 result = GM_AmbientTextureAttribute.Sample(GM_AmbientTexture, GM_AmbientSampler, input.Texcoord);
    result *= input.Color;
    return result;
}

//--------------------------------------------------------------------------------------
// Shadow
//--------------------------------------------------------------------------------------
VS_OUTPUT VS_Shadow( VS_INPUT input )
{
    VS_OUTPUT output;
    output.Position = GM_ToFloat4(input.Position);

    if (GM_UseAnimation == GM_SkeletalAnimation)
    {
        matrix boneTransform = GM_Bones[input.BoneIDs[0]] * input.Weights[0];
        boneTransform += GM_Bones[input.BoneIDs[1]] * input.Weights[1];
        boneTransform += GM_Bones[input.BoneIDs[2]] * input.Weights[2];
        boneTransform += GM_Bones[input.BoneIDs[3]] * input.Weights[3];
        output.Position = mul(output.Position, boneTransform);
    }
    else if (GM_UseAnimation == GM_AffineAnimation)
    {
        // 将仿射变换的值放到第1个Bones矩阵中
        output.Position = mul(output.Position, GM_Bones[0]);
    }
    else
    {
    }
    
    output.Position = mul(output.Position, GM_WorldMatrix);
    output.WorldPos = output.Position;
    output.Position = mul(output.Position, GM_ShadowInfo.ShadowMatrix[GM_ShadowInfo.CurrentCascadeLevel]);

    return output;
}

//--------------------------------------------------------------------------------------
// Filter
//--------------------------------------------------------------------------------------
int GM_KernelDeltaX = 0, GM_KernelDeltaY = 0;
float3 GM_BlendFactor = float3(1, 1, 1);
typedef float GMKernel[9];
interface IFilter
{
    float4 Sample(Texture2D tex, int3 coord);
    float4 SampleMSAA(Texture2DMS<float4> tex, int2 coord, int samplerId);
};

float4 Kernel(GMKernel kernel, Texture2D tex, int3 coord)
{
    int offset = 1;
    float4 sample[9];
    sample[0] = kernel[0] * tex.Load(int3(coord.x - GM_KernelDeltaX, coord.y - GM_KernelDeltaY, 0));
    sample[1] = kernel[1] * tex.Load(int3(coord.x, coord.y - GM_KernelDeltaY, 0));
    sample[2] = kernel[2] * tex.Load(int3(coord.x + GM_KernelDeltaX, coord.y - GM_KernelDeltaY, 0));
    sample[3] = kernel[3] * tex.Load(int3(coord.x - GM_KernelDeltaX, coord.y, 0));
    sample[4] = kernel[4] * tex.Load(int3(coord.x, coord.y, 0));
    sample[5] = kernel[5] * tex.Load(int3(coord.x + GM_KernelDeltaX, coord.y, 0));
    sample[6] = kernel[6] * tex.Load(int3(coord.x - GM_KernelDeltaX, coord.y + GM_KernelDeltaY, 0));
    sample[7] = kernel[7] * tex.Load(int3(coord.x, coord.y + GM_KernelDeltaY, 0));
    sample[8] = kernel[8] * tex.Load(int3(coord.x + GM_KernelDeltaX, coord.y + GM_KernelDeltaY, 0));

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

float4 Kernel(GMKernel kernel, Texture2DMS<float4> tex, int2 coord, int samplerId)
{
    int offset = 1;
    float4 sample[9];
    sample[0] = kernel[0] * tex.Load(int2(coord.x - GM_KernelDeltaX, coord.y - GM_KernelDeltaY), samplerId);
    sample[1] = kernel[1] * tex.Load(int2(coord.x, coord.y - GM_KernelDeltaY), samplerId);
    sample[2] = kernel[2] * tex.Load(int2(coord.x + GM_KernelDeltaX, coord.y - GM_KernelDeltaY), samplerId);
    sample[3] = kernel[3] * tex.Load(int2(coord.x - GM_KernelDeltaX, coord.y), samplerId);
    sample[4] = kernel[4] * tex.Load(int2(coord.x, coord.y), samplerId);
    sample[5] = kernel[5] * tex.Load(int2(coord.x + GM_KernelDeltaX, coord.y), samplerId);
    sample[6] = kernel[6] * tex.Load(int2(coord.x - GM_KernelDeltaX, coord.y + GM_KernelDeltaY), samplerId);
    sample[7] = kernel[7] * tex.Load(int2(coord.x, coord.y + GM_KernelDeltaY), samplerId);
    sample[8] = kernel[8] * tex.Load(int2(coord.x + GM_KernelDeltaX, coord.y + GM_KernelDeltaY), samplerId);

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

    float4 SampleMSAA(Texture2DMS<float4> tex, int2 coord, int samplerId)
    {
        return tex.Load(coord, samplerId);
    }
};

class GMInversionFilter : IFilter
{
    float4 Sample(Texture2D tex, int3 coord)
    {
        return GM_ToFloat4((float3(1.f, 1.f, 1.f) - tex.Load(coord).rgb), 1);
    }

    float4 SampleMSAA(Texture2DMS<float4> tex, int2 coord, int samplerId)
    {
        return GM_ToFloat4((float3(1.f, 1.f, 1.f) - tex.Load(coord, samplerId).rgb), 1);
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

    float4 SampleMSAA(Texture2DMS<float4> tex, int2 coord, int samplerId)
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

    float4 SampleMSAA(Texture2DMS<float4> tex, int2 coord, int samplerId)
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

    float4 SampleMSAA(Texture2DMS<float4> tex, int2 coord, int samplerId)
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

    float4 SampleMSAA(Texture2DMS<float4> tex, int2 coord, int samplerId)
    {
        GMKernel kernel = {
            1, 1, 1,
            1, -8, 1,
            1, 1, 1
        };
        return Kernel(kernel, tex, coord, samplerId);
    }
};

class GMBlendFilter : IFilter
{
    float4 Sample(Texture2D tex, int3 coord)
    {
        float3 fragColor = tex.Load(coord).rgb * GM_BlendFactor;
        return float4(fragColor.x, fragColor.y, fragColor.z, 1);
    }

    float4 SampleMSAA(Texture2DMS<float4> tex, int2 coord, int samplerId)
    {
        float3 fragColor = tex.Load(coord, samplerId).rgb * GM_BlendFactor;
        return float4(fragColor.x, fragColor.y, fragColor.z, 1);
    }
};

GMDefaultFilter GM_DefaultFilter;
GMInversionFilter GM_InversionFilter;
GMSharpenFilter GM_SharpenFilter;
GMBlurFilter GM_BlurFilter;
GMGrayscaleFilter GM_GrayscaleFilter;
GMEdgeDetectFilter GM_EdgeDetectFilter;
GMBlendFilter GM_BlendFilter;

IFilter GM_Filter;
Texture2D GM_FilterTexture;
Texture2DMS<float4> GM_FilterTexture_MSAA;

VS_OUTPUT VS_Filter(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = float4(input.Position.x, input.Position.y, input.Position.z, 1);
    output.Texcoord = input.Texcoord;
    output.Z = output.Position.z;
    return output;
}

float4 PS_Filter(PS_INPUT input) : SV_TARGET
{
    float4 color;
    int3 coord = int3(input.Texcoord * float2(GM_ScreenInfo.ScreenWidth, GM_ScreenInfo.ScreenHeight), 0);
    if (GM_ScreenInfo.Multisampling)
        color = GM_Filter.SampleMSAA(GM_FilterTexture_MSAA, coord.xy, 0);
    else
        color = GM_Filter.Sample(GM_FilterTexture, coord);

    return (CalculateGammaCorrectionIfNecessary(CalculateWithToneMapping(max(color, 0))));
}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 GMTech_3D
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_3D()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0,PS_3D()));
        SetRasterizerState(GM_RasterizerState);
        SetBlendState(GM_BlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(GM_DepthStencilState, 1);
    }
}

technique11 GMTech_2D
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_2D()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0,PS_2D()));
        SetRasterizerState(GM_RasterizerState);
        SetDepthStencilState(GM_DepthStencilState, 1);
        SetBlendState(GM_BlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_Text
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_Text()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0,PS_Text()));
        SetRasterizerState(GM_RasterizerState);
        SetDepthStencilState(GM_DepthStencilState, 1);
        SetBlendState(GM_BlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_CubeMap
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_CubeMap()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0,PS_CubeMap()));
        SetRasterizerState(GM_RasterizerState);
        SetDepthStencilState(GM_DepthStencilState, 1);
        SetBlendState(GM_BlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_Filter
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_Filter()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0,PS_Filter()));
        SetRasterizerState(GM_RasterizerState);
        SetDepthStencilState(GM_DepthStencilState, 1);
        SetBlendState(GM_BlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_Deferred_3D
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_3D_GeometryPass()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0,PS_3D_GeometryPass()));
        SetRasterizerState(GM_RasterizerState);
        SetDepthStencilState(GM_DepthStencilState, 1);
        SetBlendState(GM_BlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_Deferred_3D_LightPass
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_3D_LightPass()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0,PS_3D_LightPass()));
        SetRasterizerState(GM_RasterizerState);
        SetDepthStencilState(GM_DepthStencilState, 1);
        SetBlendState(GM_BlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_3D_Shadow
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_Shadow()));
        SetPixelShader(NULL);
        SetGeometryShader(NULL);
        SetRasterizerState(GM_RasterizerState);
        SetDepthStencilState(GM_DepthStencilState, 1);
        SetBlendState(GM_BlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_Particle
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_Particle()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0,PS_Particle()));
        SetRasterizerState(GM_RasterizerState);
        SetDepthStencilState(GM_DepthStencilState, 1);
        SetBlendState(GM_BlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}