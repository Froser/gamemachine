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

struct GMTexture
{
    float OffsetX;
    float OffsetY;
    float ScaleX;
    float ScaleY;
    bool Enabled;
};

GMTexture AmbientTextureAttributes[3];
GMTexture DiffuseTextureAttributes[3];
Texture2D AmbientTexture_0: register(t0);
Texture2D AmbientTexture_1: register(t1);
Texture2D AmbientTexture_2: register(t2);
Texture2D DiffuseTexture_0: register(t3);
Texture2D DiffuseTexture_1: register(t4);
Texture2D DiffuseTexture_2: register(t5);
SamplerState AmbientSampler_0: register(s0);
SamplerState AmbientSampler_1: register(s1);
SamplerState AmbientSampler_2: register(s2);
SamplerState DiffuseSampler_0: register(s3);
SamplerState DiffuseSampler_1: register(s4);
SamplerState DiffuseSampler_2: register(s5);

struct GMLight
{
    float3 Position;
    float3 Color;
};

GMLight AmbientLights[10];
int AmbientLightCount;

GMLight SpecularLights[10];
int SpecularLightCount;

struct GMMaterial
{
    float3 Ka;
    float3 Kd;
    float3 Ks;
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
    float3 Normal      : NORMAL;
    float2 Texcoord    : TEXCOORD0;
    float2 Tangent     : TEXCOORD1;
    float2 Bitangent   : TEXCOORD2;
    float2 Lightmap    : TEXCOORD3;
    float4 Color       : COLOR;
};

struct VS_OUTPUT
{
    float3 Normal      : NORMAL;
    float2 Texcoord    : TEXCOORD0;
    float2 Tangent     : TEXCOORD1;
    float2 Bitangent   : TEXCOORD2;
    float2 Lightmap    : TEXCOORD3;
    float4 Color       : COLOR;
    float4 WorldPos    : POSITION;
    float4 Position    : SV_POSITION;
};

struct PS_INPUT
{
    float3 Normal      : NORMAL;
    float2 Texcoord    : TEXCOORD0;
    float2 Tangent     : TEXCOORD1;
    float2 Bitangent   : TEXCOORD2;
    float2 Lightmap    : TEXCOORD3;
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

float4 Texture_Sample(Texture2D tex, SamplerState ss, float2 texcoord, GMTexture attributes)
{
    if (!attributes.Enabled)
        return float4(0.0f, 0.0f, 0.0f, 0.0f);

    float2 transformedTexcoord = texcoord * float2(attributes.ScaleX, attributes.ScaleY) + float2(attributes.OffsetX, attributes.OffsetY);
    return tex.Sample(ss, transformedTexcoord);
}

bool NeedDiscard(GMTexture attributes[3])
{
    for (int i = 0; i < 3; ++i)
    {
        if (attributes[i].Enabled)
            return false;
    }
    return true;
}

struct _LightFactor
{
    float3 DiffuseFactor;
    float3 SpecularFactor;
};
//--------------------------------------------------------------------------------------
// 3D
//--------------------------------------------------------------------------------------

_LightFactor CalculateLightFactor(GMLight specular, float3 eyeDirection_Eye, float3 normal_Eye)
{
    _LightFactor result;
    float3 lightPosition_Eye = (mul(ToFloat4(specular.Position), ViewMatrix)).xyz;
    float3 lightDirection_Eye = lightPosition_Eye + eyeDirection_Eye;
    float3 L = normalize(lightDirection_Eye);

    // Diffuse
    result.DiffuseFactor = saturate(dot(L, normal_Eye)) * Material.Kd * specular.Color;

    // Specular
    float3 V = normalize(eyeDirection_Eye);
    float3 R = reflect(-L, normal_Eye);
    float theta = dot(V, R);
    float factor_Specular = pow(abs(theta), Material.Shininess);
    factor_Specular = saturate(factor_Specular);
    result.SpecularFactor = factor_Specular * Material.Ks * specular.Color;

    return result;
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

float4 PS_3D(PS_INPUT input) : SV_Target
{
    //TODO 没有纹理，真的不需要绘制吗？
    if (NeedDiscard(DiffuseTextureAttributes) && NeedDiscard(AmbientTextureAttributes))
        discard;

    _LightFactor lightFactor;
    lightFactor.DiffuseFactor = 0;
    lightFactor.SpecularFactor = 0;

    // 将法线换算到眼睛坐标系
    matrix transform_Normal_Eye = mul(InverseTransposeModelMatrix, ViewMatrix);
    float3 normal_Eye = normalize( mul(ToFloat4(input.Normal.xyz, 0), transform_Normal_Eye).xyz);
    float3 position_Eye = (mul(input.WorldPos, ViewMatrix)).xyz;
    float3 eyeDirection_Eye = -position_Eye;

    for (int i = 0; i < SpecularLightCount; ++i)
    {
        _LightFactor t = CalculateLightFactor(SpecularLights[i], eyeDirection_Eye, normal_Eye);
        lightFactor.DiffuseFactor += t.DiffuseFactor;
        lightFactor.SpecularFactor += t.SpecularFactor;
    }

    // 计算Ambient因子
    float4 color_Ambient = float4(0, 0, 0, 0);
    float4 factor_Ambient = float4(0, 0, 0, 0);
    for (int j = 0; j < AmbientLightCount; ++j)
    {
        factor_Ambient += ToFloat4(Material.Ka * AmbientLights[j].Color);
    }
    color_Ambient += Texture_Sample(AmbientTexture_0, AmbientSampler_0, input.Texcoord, AmbientTextureAttributes[0]);
    color_Ambient += Texture_Sample(AmbientTexture_1, AmbientSampler_1, input.Texcoord, AmbientTextureAttributes[1]);
    color_Ambient += Texture_Sample(AmbientTexture_2, AmbientSampler_2, input.Texcoord, AmbientTextureAttributes[2]);
    color_Ambient = factor_Ambient * color_Ambient;

    // 计算Diffuse因子
    float4 color_Diffuse = float4(0, 0, 0, 0);
    color_Diffuse += Texture_Sample(DiffuseTexture_0, DiffuseSampler_0, input.Texcoord, DiffuseTextureAttributes[0]);
    color_Diffuse += Texture_Sample(DiffuseTexture_1, DiffuseSampler_1, input.Texcoord, DiffuseTextureAttributes[1]);
    color_Diffuse += Texture_Sample(DiffuseTexture_2, DiffuseSampler_2, input.Texcoord, DiffuseTextureAttributes[2]);
    color_Diffuse = ToFloat4(lightFactor.DiffuseFactor) * color_Diffuse;
    
    float4 finalColor = color_Ambient + color_Diffuse + ToFloat4(lightFactor.SpecularFactor);
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
    if (NeedDiscard(DiffuseTextureAttributes) && NeedDiscard(AmbientTextureAttributes))
        return float4(0, 0, 0, 0);

    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    color += Texture_Sample(AmbientTexture_0, AmbientSampler_0, input.Texcoord, AmbientTextureAttributes[0]);
    color += Texture_Sample(AmbientTexture_1, AmbientSampler_1, input.Texcoord, AmbientTextureAttributes[1]);
    color += Texture_Sample(AmbientTexture_2, AmbientSampler_2, input.Texcoord, AmbientTextureAttributes[2]);
    color += Texture_Sample(DiffuseTexture_0, DiffuseSampler_0, input.Texcoord, DiffuseTextureAttributes[0]);
    color += Texture_Sample(DiffuseTexture_1, DiffuseSampler_1, input.Texcoord, DiffuseTextureAttributes[1]);
    color += Texture_Sample(DiffuseTexture_2, DiffuseSampler_2, input.Texcoord, DiffuseTextureAttributes[2]);
    return color;
}

float4 PS_Glyph(PS_INPUT input) : SV_Target
{
    float4 alpha = AmbientTexture_0.Sample(AmbientSampler_0, input.Texcoord);
    return float4(input.Color.r, input.Color.g, input.Color.b, alpha.r);
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
