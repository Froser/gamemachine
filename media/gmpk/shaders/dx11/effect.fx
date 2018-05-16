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
    matrix GM_WorldMatrix;
    matrix GM_ViewMatrix;
    matrix GM_ProjectionMatrix;
    matrix GM_InverseTransposeModelMatrix;
    matrix GM_InverseViewMatrix;
    float4 GM_ViewPosition;
}

//--------------------------------------------------------------------------------------
// Textures, LightAttributes, Materials
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
        return TextureRGBToNormal(tex, ss, texcoord);
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
int GM_LightCount;
GMLightProxy LightProxy;

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
float GM_GammaInv;
float4 CalculateGammaCorrection(float4 factor)
{
    if (!GM_GammaCorrection)
        return factor;

    return ToFloat4(pow(factor.rgb, float3(GM_GammaInv, GM_GammaInv, GM_GammaInv)));
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
        return ToFloat4(GM_ToneMapping.ToneMapping(c.rgb));
    return c;
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
Texture2D GM_ShadowMap;
Texture2DMS<float4> GM_ShadowMapMSAA;

GMShadowInfo GM_ShadowInfo;
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
        Normal_Tangent_N = normalMap.RGBToNormal(GM_NormalMapTexture, GM_NormalMapSampler, texcoord);
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
    return cubeMap.Sample(tex, GM_CubeMapSampler, refraction.xyz);
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
        float3 normalFromTangent_World_N = mul(normal_Eye_N, ToFloat3x3(GM_InverseViewMatrix));
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
    float3 GM_DiffuseTexture;
    float3 GM_SpecularTexture;
    float Shininess;
    float Refractivity; 
};

interface IIlluminationModel
{
    float4 Calculate(PS_3D_INPUT input, float shadowFactor);
};
IIlluminationModel GM_IlluminationModel;

float CalculateShadow(matrix shadowMatrix, float4 worldPos, float3 normal_N)
{
    if (!GM_ShadowInfo.HasShadow)
        return 1.0f;

    float4 fragPos = mul(worldPos, shadowMatrix);
    float3 projCoords = fragPos.xyz / fragPos.w;
    if (projCoords.z > 1.0f)
        return 1.0f;

    projCoords.x = projCoords.x * 0.5f + 0.5f;
    projCoords.y = projCoords.y * (-0.5f) + 0.5f;

    float bias = (GM_ShadowInfo.BiasMin == GM_ShadowInfo.BiasMax) ? GM_ShadowInfo.BiasMin : max(GM_ShadowInfo.BiasMax * (1.0 - dot(normal_N, normalize(worldPos.xyz - GM_ShadowInfo.Position.xyz))), GM_ShadowInfo.BiasMin);
    float closestDepth = 0;
    if (GM_ScreenInfo.Multisampling)
    {
        int x = GM_ShadowInfo.ShadowMapWidth * projCoords.x;
        int y = GM_ShadowInfo.ShadowMapHeight * projCoords.y;
        if (projCoords.x > 1 || projCoords.x < 0 ||
            projCoords.y > 1 || projCoords.y < 0 )
        {
            return 1.f;
        }

        closestDepth = GM_ShadowMapMSAA.Load(int3(x, y, 0), 0);
    }
    else
    {
        closestDepth = GM_ShadowMap.Sample(ShadowMapSampler, projCoords.xy).r;
    }

    return projCoords.z - bias > closestDepth ? 0.f : 1.f;
}

class GMPhong : IIlluminationModel
{
    float4 Calculate(PS_3D_INPUT input, float shadowFactor)
    {
        float4 factor_Ambient = float4(0, 0, 0, 0);
        float4 factor_Diffuse = float4(0, 0, 0, 0);
        float4 factor_Specular = float4(0, 0, 0, 0);
        input.AmbientLightmapTexture = max(input.AmbientLightmapTexture, float3(0, 0, 0));
        input.GM_DiffuseTexture = max(input.GM_DiffuseTexture, float3(0, 0, 0));
        input.GM_SpecularTexture = max(input.GM_SpecularTexture, float3(0, 0, 0));

        // 将法线换算到眼睛坐标系
        float3 position_Eye = (mul(ToFloat4(input.WorldPos), GM_ViewMatrix)).xyz;
        float3 position_Eye_N = normalize(position_Eye);

        for (int i = 0; i < GM_LightCount; ++i)
        {
            float3 lightPosition_Eye = GetLightPositionInEyeSpace(LightAttributes[i].Position, GM_ViewMatrix);
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
        float4 color_Ambient = CalculateGammaCorrection(factor_Ambient) * ToFloat4(input.AmbientLightmapTexture);
        float4 color_Diffuse = shadowFactor * CalculateGammaCorrection(factor_Diffuse) * ToFloat4(input.GM_DiffuseTexture);

        // 计算Specular
        float4 color_Specular = shadowFactor * CalculateGammaCorrection(factor_Specular) * ToFloat4(input.GM_SpecularTexture);
        
        // 计算折射
        float4 color_Refractivity = IlluminateRefraction(
            GM_CubeMapTextureAttribute,
            GM_CubeMapTexture,
            input.Normal_World_N,
            ToFloat4(input.WorldPos),
            GM_ViewPosition,
            input.HasNormalMap,
            input.TangentSpace,
            input.Refractivity
            );
        color_Refractivity = color_Refractivity;

        return color_Ambient + color_Diffuse + color_Specular + color_Refractivity;
    }
};
GMPhong GM_Phong;

float4 PS_3D_CalculateColor(PS_3D_INPUT input)
{
    float factor_Shadow = CalculateShadow(GM_ShadowInfo.ShadowMatrix, ToFloat4(input.WorldPos), input.Normal_World_N);
    return GM_IlluminationModel.Calculate(input, factor_Shadow);
}

VS_OUTPUT VS_3D( VS_INPUT input )
{
    VS_OUTPUT output;
    output.Position = ToFloat4(input.Position);
    output.Position = mul(output.Position, GM_WorldMatrix);
    output.WorldPos = output.Position;
    
    output.Position = mul(output.Position, GM_ViewMatrix);
    output.Position = mul(output.Position, GM_ProjectionMatrix);

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
    return GM_NormalMapTextureAttribute.Enabled;
}

GMTexture PS_3D_NormalMap()
{
    return GM_NormalMapTextureAttribute;
}

float4 PS_3D(PS_INPUT input) : SV_TARGET
{
    // 将法线换算到眼睛坐标系
    float3x3 inverseTransposeModelMatrix = ToFloat3x3(GM_InverseTransposeModelMatrix);
    float3x3 transform_Normal_Eye = mul(inverseTransposeModelMatrix, ToFloat3x3(GM_ViewMatrix));
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
    float4 color_Ambient = GM_AmbientTextureAttribute.Sample(GM_AmbientTexture, GM_AmbientSampler, input.Texcoord);
    color_Ambient *= GM_LightmapTextureAttribute.Sample(GM_LightmapTexture, GM_LightmapSampler, input.Lightmap);

    // 计算Diffuse
    float4 color_Diffuse = GM_DiffuseTextureAttribute.Sample(GM_DiffuseTexture, GM_DiffuseSampler, input.Texcoord);

    // 计算Specular(如果有Specular贴图)
    float4 color_Specular = GM_SpecularTextureAttribute.Sample(GM_SpecularTexture, GM_SpecularSampler, input.Texcoord).r;

    commonInput.AmbientLightmapTexture = color_Ambient * GM_Material.Ka;
    commonInput.GM_DiffuseTexture = color_Diffuse * GM_Material.Kd;
    commonInput.GM_SpecularTexture = color_Specular * GM_Material.Ks;
    commonInput.Shininess = GM_Material.Shininess;
    commonInput.Refractivity = GM_Material.Refractivity;

    return PS_3D_CalculateColor(commonInput);
}

//--------------------------------------------------------------------------------------
// 2D
//--------------------------------------------------------------------------------------
VS_OUTPUT VS_Flat(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = float4(input.Position.x, input.Position.y, input.Position.z, 1);
    output.Position = mul(output.Position, GM_WorldMatrix);
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
    if (!GM_DiffuseTextureAttribute.Enabled && !GM_AmbientTextureAttribute.Enabled)
        return float4(0, 0, 0, 0);

    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    color += GM_AmbientTextureAttribute.Sample(GM_AmbientTexture, GM_AmbientSampler, input.Texcoord);
    color += GM_DiffuseTextureAttribute.Sample(GM_DiffuseTexture, GM_DiffuseSampler, input.Texcoord);
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
    float4 alpha = GM_AmbientTexture.Sample(GM_AmbientSampler, input.Texcoord);
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
    output.Position = mul(output.Position, GM_WorldMatrix);
    output.Position = mul(output.Position, GM_ViewMatrix);
    output.Position = mul(output.Position, GM_ProjectionMatrix);
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
Texture2D GM_DeferredNormal_World;
Texture2D GM_DeferredTextureAmbient;
Texture2D GM_DeferredTextureDiffuse;
Texture2D GM_DeferredTangent_Eye;
Texture2D GM_DeferredBitangent_Eye;
Texture2D GM_DeferredNormalMap_bNormalMap;
Texture2D GM_DeferredSpecular_Shininess;

Texture2DMS<float4> GM_DeferredPosition_World_Refractivity_MSAA;
Texture2DMS<float4> GM_DeferredNormal_World_MSAA;
Texture2DMS<float4> GM_DeferredTextureAmbient_MSAA;
Texture2DMS<float4> GM_DeferredTextureDiffuse_MSAA;
Texture2DMS<float4> GM_DeferredTangent_Eye_MSAA;
Texture2DMS<float4> GM_DeferredBitangent_Eye_MSAA;
Texture2DMS<float4> GM_DeferredNormalMap_bNormalMap_MSAA;
Texture2DMS<float4> GM_DeferredSpecular_Shininess_MSAA;

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
    float4 Specular_Shininess                : SV_TARGET7;
};

VS_OUTPUT VS_3D_GeometryPass(VS_INPUT input)
{
    return VS_3D(input);
}

VS_GEOMETRY_OUTPUT PS_3D_GeometryPass(PS_INPUT input)
{
    VS_GEOMETRY_OUTPUT output;
    output.Position_Refractivity.rgb = input.WorldPos;
    output.Position_Refractivity.a = GM_Material.Refractivity;

    float3x3 inverseTransposeModelMatrix = ToFloat3x3(GM_InverseTransposeModelMatrix);
    float3x3 normalEyeTransform = mul(inverseTransposeModelMatrix, ToFloat3x3(GM_ViewMatrix));
    float4 normal_Model = ToFloat4(input.Normal.xyz, 0);
    output.Normal_World = Float3ToTexture( normalize(mul(input.Normal.xyz, inverseTransposeModelMatrix)) );

    float4 texAmbient = GM_AmbientTextureAttribute.Sample(GM_AmbientTexture, GM_AmbientSampler, input.Texcoord);
    texAmbient *= GM_LightmapTextureAttribute.Sample(GM_LightmapTexture, GM_LightmapSampler, input.Lightmap);
    
    float4 texDiffuse = GM_DiffuseTextureAttribute.Sample(GM_DiffuseTexture, GM_DiffuseSampler, input.Texcoord);
    output.TextureAmbient = texAmbient * GM_Material.Ka;
    output.TextureDiffuse = texDiffuse * GM_Material.Kd;

    float texSpecular = GM_SpecularTextureAttribute.Sample(GM_SpecularTexture, GM_SpecularSampler, input.Texcoord).r;

    if (PS_3D_HasNormalMap())
    {
        output.Tangent_Eye = Float3ToTexture(normalize(mul(input.Tangent, normalEyeTransform)));
        output.Bitangent_Eye = Float3ToTexture(normalize(mul(input.Bitangent, normalEyeTransform)));
        output.NormalMap_HasNormalMap = ToFloat4(PS_3D_NormalMap().Sample(GM_NormalMapTexture, GM_NormalMapSampler, input.Texcoord), 1);
    }
    else
    {
        output.Tangent_Eye = Float3ToTexture(float4(0, 0, 0, 0));
        output.Bitangent_Eye = Float3ToTexture(float4(0, 0, 0, 0));
        output.NormalMap_HasNormalMap = float4(0, 0, 0, 0);
    }

    output.Specular_Shininess = ToFloat4(GM_Material.Ks * texSpecular, GM_Material.Shininess);
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
    int3 coord = int3(input.Texcoord * float2(GM_ScreenInfo.ScreenWidth, GM_ScreenInfo.ScreenHeight), 0);
    TangentSpace tangentSpace;
    float3 tangent_Eye_N;
    float3 bitangent_Eye_N;
    float4 PosRef;
    float4 KSS;
    float4 normalMapFlag;
    if (!GM_ScreenInfo.Multisampling)
    {
        tangent_Eye_N = TextureRGBToNormal(GM_DeferredTangent_Eye, coord).rgb;
        bitangent_Eye_N = TextureRGBToNormal(GM_DeferredBitangent_Eye, coord).rgb;
        PosRef = GM_DeferredPosition_World_Refractivity.Load(coord);
        commonInput.Normal_World_N = TextureRGBToNormal(GM_DeferredNormal_World, coord);
        commonInput.AmbientLightmapTexture = (GM_DeferredTextureAmbient.Load(coord)).rgb;
        commonInput.GM_DiffuseTexture = (GM_DeferredTextureDiffuse.Load(coord)).rgb;
        KSS = GM_DeferredSpecular_Shininess.Load(coord);
        normalMapFlag = GM_DeferredNormalMap_bNormalMap.Load(coord);
    }
    else
    {
        tangent_Eye_N = TextureRGBToNormal(GM_DeferredTangent_Eye_MSAA, coord).rgb;
        bitangent_Eye_N = TextureRGBToNormal(GM_DeferredBitangent_Eye_MSAA, coord).rgb;
        PosRef = GM_DeferredPosition_World_Refractivity_MSAA.Load(coord, 0);
        commonInput.Normal_World_N = TextureRGBToNormal(GM_DeferredNormal_World_MSAA, coord);
        commonInput.AmbientLightmapTexture = (GM_DeferredTextureAmbient_MSAA.Load(coord, 0)).rgb;
        commonInput.GM_DiffuseTexture = (GM_DeferredTextureDiffuse_MSAA.Load(coord, 0)).rgb;
        KSS = GM_DeferredSpecular_Shininess_MSAA.Load(coord, 0);
        normalMapFlag = GM_DeferredNormalMap_bNormalMap_MSAA.Load(coord, 0);
    }

    commonInput.GM_SpecularTexture = KSS.rgb;
    commonInput.Shininess = KSS.a;
    commonInput.WorldPos = PosRef.rgb;
    commonInput.Refractivity = PosRef.a;
    commonInput.HasNormalMap = (normalMapFlag.a != 0);
    commonInput.Normal_Eye_N = mul(commonInput.Normal_World_N, ToFloat3x3(GM_ViewMatrix));
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
    output.Position = mul(output.Position, GM_WorldMatrix);
    output.WorldPos = output.Position;
    
    output.Position = mul(output.Position, GM_ShadowInfo.ShadowMatrix);

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
int GM_KernelDeltaX = 0, GM_KernelDeltaY = 0;
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

float4 Kernel(GMKernel kernel, Texture2DMS<float4> tex, int3 coord, int samplerId)
{
    int offset = 1;
    float4 sample[9];
    sample[0] = kernel[0] * tex.Load(int3(coord.x - GM_KernelDeltaX, coord.y - GM_KernelDeltaY, 0), samplerId);
    sample[1] = kernel[1] * tex.Load(int3(coord.x, coord.y - GM_KernelDeltaY, 0), samplerId);
    sample[2] = kernel[2] * tex.Load(int3(coord.x + GM_KernelDeltaX, coord.y - GM_KernelDeltaY, 0), samplerId);
    sample[3] = kernel[3] * tex.Load(int3(coord.x - GM_KernelDeltaX, coord.y, 0), samplerId);
    sample[4] = kernel[4] * tex.Load(int3(coord.x, coord.y, 0), samplerId);
    sample[5] = kernel[5] * tex.Load(int3(coord.x + GM_KernelDeltaX, coord.y, 0), samplerId);
    sample[6] = kernel[6] * tex.Load(int3(coord.x - GM_KernelDeltaX, coord.y + GM_KernelDeltaY, 0), samplerId);
    sample[7] = kernel[7] * tex.Load(int3(coord.x, coord.y + GM_KernelDeltaY, 0), samplerId);
    sample[8] = kernel[8] * tex.Load(int3(coord.x + GM_KernelDeltaX, coord.y + GM_KernelDeltaY, 0), samplerId);

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

GMDefaultFilter GM_DefaultFilter;
GMInversionFilter GM_InversionFilter;
GMSharpenFilter GM_SharpenFilter;
GMBlurFilter GM_BlurFilter;
GMGrayscaleFilter GM_GrayscaleFilter;
GMEdgeDetectFilter GM_EdgeDetectFilter;

IFilter GM_Filter;
Texture2D GM_FilterTexture;
Texture2DMS<float4> GM_FilterTexture_MSAA;

VS_OUTPUT VS_Filter(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = float4(input.Position.x, input.Position.y, input.Position.z, 1);
    output.Texcoord = input.Texcoord;
    return output;
}

float4 PS_Filter(PS_INPUT input) : SV_TARGET
{
    float4 color;
    int3 coord = int3(input.Texcoord * float2(GM_ScreenInfo.ScreenWidth, GM_ScreenInfo.ScreenHeight), 0);
    if (GM_ScreenInfo.Multisampling)
        color = GM_Filter.SampleMSAA(GM_FilterTexture_MSAA, coord, 0);
    else
        color = GM_Filter.Sample(GM_FilterTexture, coord);

    return (CalculateGammaCorrection(CalculateWithToneMapping(max(color, 0))));
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
        SetPixelShader(CompileShader(ps_5_0,PS_2D()));
        SetRasterizerState(GM_RasterizerState);
        SetDepthStencilState(GM_DepthStencilState, 1);
        SetBlendState(GM_BlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_Glyph
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0,VS_Glyph()));
        SetPixelShader(CompileShader(ps_5_0,PS_Glyph()));
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
        SetRasterizerState(GM_RasterizerState);
        SetDepthStencilState(GM_DepthStencilState, 1);
        SetBlendState(GM_BlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}