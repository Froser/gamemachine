Texture2D DiffuseTex;
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
    return DiffuseTex.Sample( SamLinear, input.texcoord);
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
    return DiffuseTex.Sample( SamLinear, input.texcoord);
}

float4 PS_Glyph( PS_INPUT input ) : SV_Target
{
    float4 alpha = DiffuseTex.Sample( SamLinear, input.texcoord);
    return alpha;
}

// Techniques
technique11 GMTech_3D
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0,VS_3D() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader(ps_4_0,PS_3D() ) );
    }
}

technique11 GMTech_2D
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0,VS_2D() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader(ps_4_0,PS_2D() ) );
    }
}

technique11 GMTech_Glyph
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0,VS_2D() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader(ps_4_0,PS_Glyph() ) );
    }
}
