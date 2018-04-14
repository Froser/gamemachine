//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer WorldConstantBuffer: register( b0 ) 
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix InverseTransposeModelMatrix;
}

//--------------------------------------------------------------------------------------
// Textures, Lights, Materials
//--------------------------------------------------------------------------------------

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
        return tex.Sample(ss, texcoord).xyz * 2.0f - 1.0f;
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

GMTexture AmbientTextureAttributes[3];
GMTexture DiffuseTextureAttributes[3];
GMTexture NormalMapTextureAttributes[1];
GMLightmapTexture LightmapTextureAttributes[1];
GMTexture CubeMapTextureAttributes[1];

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

struct GMMaterial
{
    float4 Ka;
    float4 Kd;
    float4 Ks;
    float Shininess;
    float Refractivity;
};
GMMaterial Material;

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

struct PS_INPUT
{
    float3 Normal      : NORMAL0;
    float2 Texcoord    : TEXCOORD0;
    float3 Tangent     : NORMAL1;
    float3 Bitangent   : NORMAL2;
    float2 Lightmap    : TEXCOORD1;
    float4 Color       : COLOR;
    float4 WorldPos    : POSITION;
};

float4 ToFloat4(float3 v, float w)
{
    return float4(v.x, v.y, v.z, w);
}

float4 ToFloat4(float3 v)
{
    return ToFloat4(v, 1);
}

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

class NormapMapArgs
{
    float3 normal_Tangent_N;
    float3x3 TBN;

    void CalculateNormapMapArgsInEyeSpace(float2 texcoord, float3 tangent, float3 bitangent, float3 normal_Eye_N, matrix transform_Normal_Eye)
    {
        if (!HasNormalMap())
            return;

        normal_Tangent_N = NormalMap().RGBToNormal(NormalMapTexture, NormalMapSampler, texcoord);
        float3 tangent_Eye_N = normalize(mul(ToFloat4(tangent, 0), transform_Normal_Eye).xyz);
        float3 bitangent_Eye_N = normalize(mul(ToFloat4(bitangent, 0), transform_Normal_Eye).xyz);
        TBN = transpose(float3x3(
            tangent_Eye_N,
            bitangent_Eye_N,
            normal_Eye_N
        ));
    }
};
//--------------------------------------------------------------------------------------
// 3D
//--------------------------------------------------------------------------------------

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

float4 PS_3D(PS_INPUT input) : SV_Target
{
    float4 factor_Ambient = float4(0, 0, 0, 0);
    float4 factor_Diffuse = float4(0, 0, 0, 0);
    float4 factor_Specular = float4(0, 0, 0, 0);

    // 将法线换算到眼睛坐标系
    matrix transform_Normal_Eye = mul(InverseTransposeModelMatrix, ViewMatrix);
    float3 normal_Eye_N = normalize( mul(ToFloat4(input.Normal.xyz, 0), transform_Normal_Eye).xyz);
    float3 position_Eye = (mul(input.WorldPos, ViewMatrix)).xyz;
    float3 position_Eye_N = normalize(position_Eye);

    int i = 0;
    for (i = 0; i < AmbientLightCount; ++i)
    {
        factor_Ambient += AmbientLights[i].IlluminateAmbient();
    }

    NormapMapArgs normalMapArgs;
    normalMapArgs.CalculateNormapMapArgsInEyeSpace(input.Texcoord, input.Tangent, input.Bitangent, normal_Eye_N, transform_Normal_Eye);

    for (i = 0; i < SpecularLightCount; ++i)
    {
        if (!HasNormalMap())
        {
            float3 lightPosition_Eye = SpecularLights[i].GetLightPositionInEyeSpace();
            float3 lightDirection_Eye_N = normalize(lightPosition_Eye - position_Eye);
            factor_Diffuse += SpecularLights[i].IlluminateDiffuse(lightDirection_Eye_N, normal_Eye_N);
            factor_Specular += SpecularLights[i].IlluminateSpecular(lightDirection_Eye_N, -position_Eye_N, normal_Eye_N, Material.Shininess);    
        }
        else
        {
            float3 lightPosition_Eye = SpecularLights[i].GetLightPositionInEyeSpace();
            float3 lightDirection_Eye = lightPosition_Eye - position_Eye;

            float3 lightDirection_Tangent_N = normalize(mul(lightDirection_Eye, normalMapArgs.TBN));
            float3 eyeDirection_Tangent_N = normalize(mul(-position_Eye, normalMapArgs.TBN));

            factor_Diffuse += SpecularLights[i].IlluminateDiffuse(lightDirection_Tangent_N, normalMapArgs.normal_Tangent_N);
            factor_Specular += SpecularLights[i].IlluminateSpecular(lightDirection_Tangent_N, eyeDirection_Tangent_N, normalMapArgs.normal_Tangent_N, Material.Shininess); 

        }
    }

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
    color_Ambient = factor_Ambient * color_Ambient * Material.Ka;

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
    color_Diffuse = factor_Diffuse * color_Diffuse * Material.Kd;

    // 计算Specular
    float4 color_Specular = factor_Specular * Material.Ks;
    
    float4 finalColor = color_Ambient + color_Diffuse + color_Specular;
    return finalColor;
}

//--------------------------------------------------------------------------------------
// 2D
//--------------------------------------------------------------------------------------
VS_OUTPUT VS_2D(VS_INPUT input)
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

float4 PS_2D(PS_INPUT input) : SV_Target
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

float4 PS_Glyph(PS_INPUT input) : SV_Target
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
    output.Position = mul(output.Position, WorldMatrix);
    output.Position = mul(output.Position, ViewMatrix);
    output.Position = mul(output.Position, ProjectionMatrix);
    output.Texcoord = input.Texcoord;
    return output;
}

float4 PS_CubeMap(PS_INPUT input) : SV_Target
{
    return AmbientTextureAttributes[0].Sample(AmbientTexture_0, AmbientSampler_0, input.Texcoord);
}

// Techniques
technique11 GMTech_3D
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0,VS_3D()));
        SetPixelShader(CompileShader(ps_4_0,PS_3D()));
        SetRasterizerState(GMRasterizerState);
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(GMDepthStencilState, 1);
    }
}

technique11 GMTech_2D
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0,VS_2D()));
        SetPixelShader(CompileShader(ps_4_0,PS_2D()));
        SetRasterizerState(GMRasterizerState);
        SetDepthStencilState(GMDepthStencilState, 1);
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_Glyph
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0,VS_2D()));
        SetPixelShader(CompileShader(ps_4_0,PS_Glyph()));
        SetRasterizerState(GMRasterizerState);
        SetDepthStencilState(GMDepthStencilState, 1);
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_CubeMap
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0,VS_CubeMap()));
        SetPixelShader(CompileShader(ps_4_0,PS_CubeMap()));
        SetRasterizerState(GMRasterizerState);
        SetDepthStencilState(GMDepthStencilState, 1);
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}
