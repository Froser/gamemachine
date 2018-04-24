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
// Textures, Lights, Materials
//--------------------------------------------------------------------------------------
Texture2D AmbientTexture_0: register(t0);
Texture2D AmbientTexture_1: register(t1);
Texture2D AmbientTexture_2: register(t2);
Texture2D DiffuseTexture_0: register(t3);
Texture2D DiffuseTexture_1: register(t4);
Texture2D DiffuseTexture_2: register(t5);
Texture2D NormalMapTexture: register(t6);
Texture2D LightmapTexture: register(t7);
TextureCube CubeMapTexture: register(t8);
SamplerState AmbientSampler_0: register(s0);
SamplerState AmbientSampler_1: register(s1);
SamplerState AmbientSampler_2: register(s2);
SamplerState DiffuseSampler_0: register(s3);
SamplerState DiffuseSampler_1: register(s4);
SamplerState DiffuseSampler_2: register(s5);
SamplerState NormalMapSampler: register(s6);
SamplerState LightmapSampler: register(s7);
SamplerState CubeMapSampler: register(s8);

float3 TextureRGBToNormal(Texture2D tex, SamplerState ss, float2 texcoord)
{
    return tex.Sample(ss, texcoord).xyz * 2.0f - 1.0f;
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
GMTexture NormalMapTextureAttributes[1];
GMLightmapTexture LightmapTextureAttributes[1];
GMCubeMapTexture CubeMapTextureAttributes[1];

interface ILight
{
    float4 IlluminateAmbient();
    float4 IlluminateDiffuse(float3 lightDirection_N, float3 normal_N);
    float4 IlluminateSpecular(float3 lightDirection_N, float3 eyeDirection_N, float3 normal_N, float shininess);
};

class GMLight : ILight
{
    float4 Position;
    float4 Color;

    float4 IlluminateAmbient()
    {
        return Color;
    }

    float4 IlluminateDiffuse(float3 lightDirection_N, float3 normal_N)
    {
        return (saturate(dot(lightDirection_N, normal_N)) * Color);
    }

    float4 IlluminateSpecular(float3 lightDirection_N, float3 eyeDirection_N, float3 normal_N, float shininess)
    {
        float3 reflection_N = reflect(-lightDirection_N, normal_N);
        float theta = dot(eyeDirection_N, reflection_N);
        float factor_Specular = saturate(pow(abs(theta), shininess));
        return factor_Specular * Color;
    }

    float3 GetLightPositionInEyeSpace()
    {
        return (mul(Position, ViewMatrix)).xyz;
    }
};

GMLight AmbientLights[10];
int AmbientLightCount;

GMLight SpecularLights[10];
int SpecularLightCount;

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

bool HasNormalMap()
{
    return NormalMapTextureAttributes[0].Enabled;
}

GMTexture NormalMap()
{
    return NormalMapTextureAttributes[0];
}

class TangentSpace
{
    float3 Normal_Tangent_N;
    float3x3 TBN;

    void CalculateTangentSpaceInEyeSpace(float2 texcoord, float3 tangent, float3 bitangent, float3 normal_Eye_N, matrix transform_Normal_Eye)
    {
        if (!HasNormalMap())
            return;

        Normal_Tangent_N = NormalMap().RGBToNormal(NormalMapTexture, NormalMapSampler, texcoord);
        float3 tangent_Eye_N = normalize(mul(ToFloat4(tangent, 0), transform_Normal_Eye).xyz);
        float3 bitangent_Eye_N = normalize(mul(ToFloat4(bitangent, 0), transform_Normal_Eye).xyz);
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
    TangentSpace tangentSpace,
    float refractivity
    )
{
    if (refractivity == 0)
        return float4(0, 0, 0, 0);

    if (HasNormalMap())
    {
        // 如果是切线空间，计算会复杂点，要将切线空间的法线换算回世界空间
        float4 normal_Eye_N = ToFloat4(mul(tangentSpace.Normal_Tangent_N, transpose(tangentSpace.TBN)), 0);
        float3 normalFromTangent_World_N = mul(normal_Eye_N, InverseViewMatrix).xyz;
        return IlluminateRefractionByNormalWorldN(cubeMap, tex, normalFromTangent_World_N, position_World, viewPosition_World, refractivity);
    }

    return IlluminateRefractionByNormalWorldN(cubeMap, tex, normal_World_N, position_World, viewPosition_World, refractivity);
}

//--------------------------------------------------------------------------------------
// 3D
//--------------------------------------------------------------------------------------

struct VS_3D_INPUT
{
    float3 WorldPos;            // 世界坐标
    float3 Normal_World_N;      // 世界法线
    float3 Normal_Eye_N;        // 眼睛空间法向量
    TangentSpace TangentSpace;  // 切线空间
    bool HasNormalMap;          // 是否有法线贴图
    float3 AmbientLightmapTexture;
    float3 DiffuseTexture;
    float3 Ks;
    float Shininess;
    float Refractivity; 
};

float4 PS_3D_Common(VS_3D_INPUT input)
{
    float4 factor_Ambient = float4(0, 0, 0, 0);
    float4 factor_Diffuse = float4(0, 0, 0, 0);
    float4 factor_Specular = float4(0, 0, 0, 0);

    // 将法线换算到眼睛坐标系
    float3 position_Eye = (mul(ToFloat4(input.WorldPos), ViewMatrix)).xyz;
    float3 position_Eye_N = normalize(position_Eye);

    int i = 0;
    for (i = 0; i < AmbientLightCount; ++i)
    {
        factor_Ambient += AmbientLights[i].IlluminateAmbient();
    }

    for (i = 0; i < SpecularLightCount; ++i)
    {
        if (!input.HasNormalMap)
        {
            float3 lightPosition_Eye = SpecularLights[i].GetLightPositionInEyeSpace();
            float3 lightDirection_Eye_N = normalize(lightPosition_Eye - position_Eye);
            factor_Diffuse += SpecularLights[i].IlluminateDiffuse(lightDirection_Eye_N, input.Normal_Eye_N);
            factor_Specular += SpecularLights[i].IlluminateSpecular(lightDirection_Eye_N, -position_Eye_N, input.Normal_Eye_N, input.Shininess);    
        }
        else
        {
            float3 lightPosition_Eye = SpecularLights[i].GetLightPositionInEyeSpace();
            float3 lightDirection_Eye = lightPosition_Eye - position_Eye;

            float3 lightDirection_Tangent_N = normalize(mul(lightDirection_Eye, input.TangentSpace.TBN));
            float3 eyeDirection_Tangent_N = normalize(mul(-position_Eye, input.TangentSpace.TBN));

            factor_Diffuse += SpecularLights[i].IlluminateDiffuse(lightDirection_Tangent_N, input.TangentSpace.Normal_Tangent_N);
            factor_Specular += SpecularLights[i].IlluminateSpecular(lightDirection_Tangent_N, eyeDirection_Tangent_N, input.TangentSpace.Normal_Tangent_N, input.Shininess); 
        }
    }

    float4 color_Ambient = factor_Ambient * ToFloat4(input.AmbientLightmapTexture);
    float4 color_Diffuse = factor_Diffuse * ToFloat4(input.DiffuseTexture);

    // 计算Specular
    float4 color_Specular = factor_Specular * ToFloat4(input.Ks);
    
    // 计算折射
    float4 color_Refractivity = IlluminateRefraction(
        CubeMapTextureAttributes[0],
        CubeMapTexture,
        input.Normal_World_N,
        ToFloat4(input.WorldPos),
        ViewPosition,
        input.TangentSpace,
        input.Refractivity
        );

    float4 finalColor = color_Ambient + color_Diffuse + color_Specular + color_Refractivity;
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

float4 PS_3D(PS_INPUT input) : SV_TARGET
{
    // 将法线换算到眼睛坐标系
    matrix transform_Normal_Eye = mul(InverseTransposeModelMatrix, ViewMatrix);
    float3 normal_Eye_N = normalize( mul(ToFloat4(input.Normal.xyz, 0), transform_Normal_Eye).xyz);

    VS_3D_INPUT commonInput;
    TangentSpace tangentSpace;
    tangentSpace.CalculateTangentSpaceInEyeSpace(input.Texcoord, input.Tangent, input.Bitangent, normal_Eye_N, transform_Normal_Eye);

    commonInput.TangentSpace = tangentSpace;
    commonInput.WorldPos = input.WorldPos;
    commonInput.HasNormalMap = HasNormalMap();

    commonInput.Normal_World_N = normalize(mul(ToFloat4(input.Normal.xyz, 0), InverseTransposeModelMatrix).xyz);
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

    commonInput.AmbientLightmapTexture = color_Ambient * Material.Ka;
    commonInput.DiffuseTexture = color_Diffuse * Material.Kd;
    commonInput.Ks = Material.Ks;
    commonInput.Shininess = Material.Shininess;
    commonInput.Refractivity = Material.Refractivity;

    return PS_3D_Common(commonInput);
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
Texture2D GM_DeferredPosition;
Texture2D GM_DeferredNormal_World;
Texture2D GM_DeferredNormal_Eye;
Texture2D GM_DeferredTextureAmbient;
Texture2D GM_DeferredTextureDiffuse;
Texture2D GM_DeferredTangent_Eye;
Texture2D GM_DeferredBitangent_Eye;
Texture2D GM_DeferredNormalMap;
Texture2D GM_DeferredKs;
Texture2D GM_DeferredShininess_bNormalMap_Refractivity;
SamplerState DeferredSampler;

// [-1, 1] -> [0, 1]
float4 Float3ToTexture(float3 normal)
{
    return ToFloat4((normal + 1) * .5f, 1);
}

struct VS_GEOMETRY_OUTPUT
{
    float4 Position                          : SV_TARGET0; //需要将区间转化到[0, 1]
    float4 Normal_World                      : SV_TARGET1; //需要将区间转化到[0, 1]
    float4 Normal_Eye                        : SV_TARGET2; //需要将区间转化到[0, 1]
    float4 TextureAmbient                    : SV_TARGET3;
    float4 TextureDiffuse                    : SV_TARGET4;
    float4 Tangent_Eye                       : SV_TARGET5; //需要将区间转化到[0, 1]
    float4 Bitangent_Eye                     : SV_TARGET6; //需要将区间转化到[0, 1]
    float4 NormalMap                         : SV_TARGET7;
};

struct VS_MATERIAL_OUTPUT
{
    float4 Ks                                : SV_TARGET0;
    float4 Shininess_bNormalMap_Refractivity : SV_TARGET1;
};

VS_OUTPUT VS_3D_GeometryPass(VS_INPUT input)
{
    return VS_3D(input);
}

VS_GEOMETRY_OUTPUT PS_3D_GeometryPass(PS_INPUT input)
{
    VS_GEOMETRY_OUTPUT output;
    output.Position = Float3ToTexture(mul(input.WorldPos, WorldMatrix));

    float4x4 normalEyeTransform = mul(InverseTransposeModelMatrix, ViewMatrix);
    float4 normal_Model = ToFloat4(input.Normal.xyz, 0);
    output.Normal_World = Float3ToTexture( normalize(mul(normal_Model, InverseTransposeModelMatrix)).xyz );
    output.Normal_Eye = Float3ToTexture( normalize(mul(normal_Model, normalEyeTransform )).xyz );

    float4 texAmbient = float4(0, 0, 0, 0);
    float4 texDiffuse = float4(0, 0, 0, 0);
    if (HasNoTexture(AmbientTextureAttributes))
    {
        texAmbient = float4(0, 0, 0, 0);
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
        texDiffuse = float4(0, 0, 0, 0);
    }
    else
    {
        texDiffuse += DiffuseTextureAttributes[0].Sample(DiffuseTexture_0, DiffuseSampler_0, input.Texcoord);
        texDiffuse += DiffuseTextureAttributes[1].Sample(DiffuseTexture_1, DiffuseSampler_1, input.Texcoord);
        texDiffuse += DiffuseTextureAttributes[2].Sample(DiffuseTexture_2, DiffuseSampler_2, input.Texcoord);
    }
    output.TextureAmbient = texAmbient * Material.Ka;
    output.TextureDiffuse = texDiffuse * Material.Kd;
    output.NormalMap = NormalMap().Sample(NormalMapTexture, NormalMapSampler, input.Texcoord);

    if (HasNormalMap())
    {
        output.Tangent_Eye = Float3ToTexture(mul(ToFloat4(input.Tangent, 0), normalEyeTransform));
        output.Bitangent_Eye = Float3ToTexture(mul(ToFloat4(input.Bitangent, 0), normalEyeTransform));
    }
    else
    {
        output.Tangent_Eye = Float3ToTexture(float4(0, 0, 0, 0));
        output.Bitangent_Eye = Float3ToTexture(float4(0, 0, 0, 0));
    }
    return output;
}

VS_OUTPUT VS_3D_MaterialPass(VS_INPUT input)
{
    return VS_3D(input);
}

VS_MATERIAL_OUTPUT PS_3D_MaterialPass(PS_INPUT input)
{
    VS_MATERIAL_OUTPUT output;
    output.Ks = Material.Ks;
    output.Shininess_bNormalMap_Refractivity.r = Material.Shininess;
    output.Shininess_bNormalMap_Refractivity.g = HasNormalMap() ? 1 : 0;
    output.Shininess_bNormalMap_Refractivity.b = Material.Refractivity;
    output.Shininess_bNormalMap_Refractivity.a = 1; //目前空缺，便于调试，设置成1
    return output;
}

VS_OUTPUT VS_3D_LightPass(VS_INPUT input)
{
    return VS_3D(input);
}

float4 PS_3D_LightPass(PS_INPUT input) : SV_TARGET
{
    float3 tangent_Eye_N = TextureRGBToNormal(GM_DeferredTangent_Eye, DeferredSampler, input.Texcoord).rgb;
    float3 bitangent_Eye_N = TextureRGBToNormal(GM_DeferredBitangent_Eye, DeferredSampler, input.Texcoord).rgb;
    float3 normal_Eye_N = TextureRGBToNormal(GM_DeferredNormal_Eye, DeferredSampler, input.Texcoord);

    TangentSpace tangentSpace;
    tangentSpace.Normal_Tangent_N = TextureRGBToNormal(GM_DeferredNormalMap, DeferredSampler, input.Texcoord);
    tangentSpace.TBN = transpose(float3x3(
        tangent_Eye_N,
        bitangent_Eye_N,
        normal_Eye_N
    ));

    VS_3D_INPUT commonInput;
    commonInput.Normal_World_N = TextureRGBToNormal(GM_DeferredNormal_World, DeferredSampler, input.Texcoord);
    commonInput.Normal_Eye_N = normal_Eye_N;
    commonInput.WorldPos = TextureRGBToNormal(GM_DeferredPosition, DeferredSampler, input.Texcoord);
    commonInput.TangentSpace = tangentSpace;

    commonInput.AmbientLightmapTexture = (GM_DeferredTextureAmbient.Sample(DeferredSampler, input.Texcoord)).rgb;
    commonInput.DiffuseTexture = (GM_DeferredTextureDiffuse.Sample(DeferredSampler, input.Texcoord)).rgb;
    commonInput.Ks = GM_DeferredKs.Sample(DeferredSampler, input.Texcoord).rgb;

    float4 SNR = GM_DeferredShininess_bNormalMap_Refractivity.Sample(DeferredSampler, input.Texcoord);
    commonInput.Shininess = SNR.r;
    commonInput.HasNormalMap = SNR.g != 0;
    commonInput.Refractivity = SNR.b;

    return PS_3D_Common(commonInput);
}

//--------------------------------------------------------------------------------------
// Filter
//--------------------------------------------------------------------------------------
float KernelDeltaX = 0.f, KernelDeltaY = 0.f;
typedef float GMKernel[9];
interface IFilter
{
    float4 Sample(Texture2D tex, SamplerState ss, float2 texcoord);
};

float4 Kernel(GMKernel kernel, Texture2D tex, SamplerState ss, float2 texcoord)
{
    int offset = 1;
    float4 sample[9];
    sample[0] = kernel[0] * tex.Sample(ss, float2(texcoord.x - KernelDeltaX, texcoord.y - KernelDeltaY));
    sample[1] = kernel[1] * tex.Sample(ss, float2(texcoord.x, texcoord.y - KernelDeltaY));
    sample[2] = kernel[2] * tex.Sample(ss, float2(texcoord.x + KernelDeltaX, texcoord.y - KernelDeltaY));
    sample[3] = kernel[3] * tex.Sample(ss, float2(texcoord.x - KernelDeltaX, texcoord.y));
    sample[4] = kernel[4] * tex.Sample(ss, float2(texcoord.x, texcoord.y));
    sample[5] = kernel[5] * tex.Sample(ss, float2(texcoord.x + KernelDeltaX, texcoord.y));
    sample[6] = kernel[6] * tex.Sample(ss, float2(texcoord.x - KernelDeltaX, texcoord.y + KernelDeltaY));
    sample[7] = kernel[7] * tex.Sample(ss, float2(texcoord.x, texcoord.y + KernelDeltaY));
    sample[8] = kernel[8] * tex.Sample(ss, float2(texcoord.x + KernelDeltaX, texcoord.y + KernelDeltaY));

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
    float4 Sample(Texture2D tex, SamplerState ss, float2 texcoord)
    {
        return tex.Sample(ss, texcoord);
    }
};

class GMInversionFilter : IFilter
{
    float4 Sample(Texture2D tex, SamplerState ss, float2 texcoord)
    {
        return ToFloat4((float3(1.f, 1.f, 1.f) - tex.Sample(ss, texcoord)).rgb, 1);
    }
};

class GMSharpenFilter : IFilter
{
    float4 Sample(Texture2D tex, SamplerState ss, float2 texcoord)
    {
        GMKernel kernel = {
            -1, -1, -1,
            -1,  9, -1,
            -1, -1, -1
        };
        return Kernel(kernel, tex, ss, texcoord);
    }
};

class GMBlurFilter : IFilter
{
    float4 Sample(Texture2D tex, SamplerState ss, float2 texcoord)
    {
        GMKernel kernel = {
            1.0 / 16, 2.0 / 16, 1.0 / 16,
            2.0 / 16, 4.0 / 16, 2.0 / 16,
            1.0 / 16, 2.0 / 16, 1.0 / 16  
        };
        return Kernel(kernel, tex, ss, texcoord);
    }
};

class GMGrayscaleFilter : IFilter
{
    float4 Sample(Texture2D tex, SamplerState ss, float2 texcoord)
    {
        float3 fragColor = tex.Sample(ss, texcoord).rgb;
        float average = 0.2126 * fragColor.r + 0.7152 * fragColor.g + 0.0722 * fragColor.b;
        return float4(average, average, average, 1);
    }
};

class GMEdgeDetectFilter : IFilter
{
    float4 Sample(Texture2D tex, SamplerState ss, float2 texcoord)
    {
        GMKernel kernel = {
            1, 1, 1,
            1, -8, 1,
            1, 1, 1
        };
        return Kernel(kernel, tex, ss, texcoord);
    }
};

GMDefaultFilter DefaultFilter;
GMInversionFilter InversionFilter;
GMSharpenFilter SharpenFilter;
GMBlurFilter BlurFilter;
GMGrayscaleFilter GrayscaleFilter;
GMEdgeDetectFilter EdgeDetectFilter;

IFilter Filter;

VS_OUTPUT VS_Filter(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = float4(input.Position.x, input.Position.y, input.Position.z, 1);
    output.Texcoord = input.Texcoord;
    return output;
}

float4 PS_Filter(PS_INPUT input) : SV_TARGET
{
    return Filter.Sample(AmbientTexture_0, AmbientSampler_0, input.Texcoord);
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

    pass P1
    {
        SetVertexShader(CompileShader(vs_5_0,VS_3D_MaterialPass()));
        SetPixelShader(CompileShader(ps_5_0,PS_3D_MaterialPass()));
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