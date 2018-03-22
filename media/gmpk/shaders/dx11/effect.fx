bool HasTex;
Texture2D Tex;

SamplerState SamLinear;

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer WorldConstantBuffer: register( b0 ) 
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
}

RasterizerState GMRasterizerState {};
BlendState GMBlendState {};

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float3 position    : POSITION;
    float3 normal      : NORMAL;
    float2 texcoord    : TEXCOORD0;
};

struct VS_OUTPUT
{
    float3 normal      : NORMAL;
    float2 texcoord    : TEXCOORD0;
    float4 position    : SV_POSITION;
};

struct PS_INPUT
{
    float3 normal      : NORMAL;
    float2 texcoord    : TEXCOORD0;
};

VS_OUTPUT VS_3D( VS_INPUT input )
{
    VS_OUTPUT output;
    output.position = float4(input.position.x, input.position.y, input.position.z, 1);
    output.position = mul(output.position, WorldMatrix);
    output.position = mul(output.position, ViewMatrix);
    output.position = mul(output.position, ProjectionMatrix);

    output.normal = input.normal;
    output.texcoord = input.texcoord;
    return output;
}

float4 PS_3D( PS_INPUT input ) : SV_Target
{
    return Tex.Sample( SamLinear, input.texcoord);
}

VS_OUTPUT VS_2D( VS_INPUT input )
{
    VS_OUTPUT output;
    output.position = float4(input.position.x, input.position.y, input.position.z, 1);
    output.position = mul(output.position, WorldMatrix);
    output.normal = input.normal;
    output.texcoord = input.texcoord;
    return output;
}

float4 PS_2D( PS_INPUT input ) : SV_Target
{
    if (!HasTex)
        discard;

    return Tex.Sample( SamLinear, input.texcoord);
}

float4 PS_Glyph( PS_INPUT input ) : SV_Target
{
    float4 alpha = Tex.Sample( SamLinear, input.texcoord);
    return float4(1, 0, 0, alpha.r);
}

// Techniques
technique11 GMTech_3D
{
    pass P0
    {
        SetRasterizerState(GMRasterizerState);
        SetVertexShader(CompileShader( vs_4_0,VS_3D() ) );
        SetPixelShader(CompileShader(ps_4_0,PS_3D() ) );
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_2D
{
    pass P0
    {
        SetRasterizerState(GMRasterizerState);
        SetVertexShader(CompileShader( vs_4_0,VS_2D() ) );
        SetPixelShader(CompileShader(ps_4_0,PS_2D() ) );
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 GMTech_Glyph
{
    pass P0
    {
        SetRasterizerState(GMRasterizerState);
        SetVertexShader(CompileShader( vs_4_0,VS_2D() ) );
        SetPixelShader(CompileShader(ps_4_0,PS_Glyph() ) );
        SetBlendState(GMBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}
