//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer WorldConstantBuffer: register( b0 ) 
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
}

struct GMTexture
{
    float OffsetX;
    float OffsetY;
    float ScaleX;
    float ScaleY;
    bool Enabled;
};

//--------------------------------------------------------------------------------------
GMTexture AmbientTextureAttributes[3];
GMTexture DiffuseTextureAttributes[3];
Texture2D DiffuseTextures[3];
SamplerState Samplers[3];
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
	if (!DiffuseTextureAttributes[0].Enabled)
		discard;

    return DiffuseTextures[0].Sample( Samplers[0], input.Texcoord);
}

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
    if (!DiffuseTextureAttributes[0].Enabled)
        discard;

    return DiffuseTextures[0].Sample( Samplers[0], input.Texcoord);
}

float4 PS_Glyph( PS_INPUT input ) : SV_Target
{
    float4 alpha = DiffuseTextures[0].Sample( Samplers[0], input.Texcoord);
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
