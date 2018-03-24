//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer WorldConstantBuffer: register( b0 ) 
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
}

//--------------------------------------------------------------------------------------
// Textures
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
//--------------------------------------------------------------------------------------
// States
//--------------------------------------------------------------------------------------
RasterizerState GMRasterizerState {};
BlendState GMBlendState {};
//--------------------------------------------------------------------------------------

struct VS_INPUT
{
    float3 Position    : POSITION;
    float3 Normal      : NORMAL;
    float2 Texcoord    : TEXCOORD0;
};

struct VS_OUTPUT
{
    float3 Normal      : NORMAL;
    float2 Texcoord    : TEXCOORD0;
    float4 Position    : SV_POSITION;
};

struct PS_INPUT
{
    float3 Normal      : NORMAL;
    float2 Texcoord    : TEXCOORD0;
};

float4 Texture_Sample(Texture2D tex, SamplerState ss, float2 texcoord, GMTexture attributes)
{
    if (!attributes.Enabled)
        return float4(0.0f, 0.0f, 0.0f, 0.0f);

    float2 transformedTexcoord = texcoord * float2(attributes.ScaleX, attributes.ScaleY) + float2(attributes.OffsetX, attributes.OffsetY);
    return tex.Sample(ss, transformedTexcoord);
}

bool needDiscard(GMTexture attributes[3])
{
    for (int i = 0; i < 3; ++i)
    {
        if (attributes[i].Enabled)
            return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------
// 3D
//--------------------------------------------------------------------------------------
VS_OUTPUT VS_3D( VS_INPUT input )
{
    VS_OUTPUT output;
    output.Position = float4(input.Position.x, input.Position.y, input.Position.z, 1);
    output.Position = mul(output.Position, WorldMatrix);
    output.Position = mul(output.Position, ViewMatrix);
    output.Position = mul(output.Position, ProjectionMatrix);

    output.Normal = input.Normal;
    output.Texcoord = input.Texcoord;
    return output;
}

float4 PS_3D(PS_INPUT input) : SV_Target
{
	if (needDiscard(DiffuseTextureAttributes))
		discard;

    float4 color = Texture_Sample(DiffuseTexture_0, DiffuseSampler_0, input.Texcoord, DiffuseTextureAttributes[0]);
    color += Texture_Sample(DiffuseTexture_1, DiffuseSampler_1, input.Texcoord, DiffuseTextureAttributes[1]);
    color += Texture_Sample(DiffuseTexture_2, DiffuseSampler_2, input.Texcoord, DiffuseTextureAttributes[2]);
    return color;
}

//--------------------------------------------------------------------------------------
// 2D
//--------------------------------------------------------------------------------------
VS_OUTPUT VS_2D( VS_INPUT input )
{
    VS_OUTPUT output;
    output.Position = float4(input.Position.x, input.Position.y, input.Position.z, 1);
    output.Position = mul(output.Position, WorldMatrix);
    output.Normal = input.Normal;
    output.Texcoord = input.Texcoord;
    return output;
}

float4 PS_2D( PS_INPUT input ) : SV_Target
{
    if (needDiscard(DiffuseTextureAttributes))
        discard;

    float4 color = Texture_Sample(DiffuseTexture_0, DiffuseSampler_0, input.Texcoord, DiffuseTextureAttributes[0]);
    color += Texture_Sample(DiffuseTexture_1, DiffuseSampler_1, input.Texcoord, DiffuseTextureAttributes[1]);
    color += Texture_Sample(DiffuseTexture_2, DiffuseSampler_2, input.Texcoord, DiffuseTextureAttributes[2]);
    return color;
}

float4 PS_Glyph( PS_INPUT input ) : SV_Target
{
    float4 alpha = DiffuseTexture_0.Sample(DiffuseSampler_0, input.Texcoord);
    return float4(1, 0, 0, alpha.r);
}

// Techniques
technique11 GMTech_3D
{
    pass P0
    {
        SetRasterizerState(GMRasterizerState);
        SetVertexShader(CompileShader(vs_4_0,VS_3D() ) );
        SetPixelShader(CompileShader(ps_4_0,PS_3D() ) );
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_2D
{
    pass P0
    {
        SetRasterizerState(GMRasterizerState);
        SetVertexShader(CompileShader(vs_4_0,VS_2D() ) );
        SetPixelShader(CompileShader(ps_4_0,PS_2D() ) );
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_Glyph
{
    pass P0
    {
        SetRasterizerState(GMRasterizerState);
        SetVertexShader(CompileShader(vs_4_0,VS_2D() ) );
        SetPixelShader(CompileShader(ps_4_0,PS_Glyph() ) );
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}
