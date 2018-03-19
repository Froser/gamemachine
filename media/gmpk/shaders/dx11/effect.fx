Texture2D diffuseTex;
SamplerState samLinear;

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
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

VS_OUTPUT VS( VS_INPUT input )
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

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 PS( PS_INPUT input ) : SV_Target
{
  return diffuseTex.Sample( samLinear, input.texcoord);
}

technique11 BasicTech
{
  pass P0
  {
    SetVertexShader( CompileShader( vs_4_0,VS() ) );
    SetGeometryShader( NULL );
    SetPixelShader( CompileShader(ps_4_0,PS() ) );
  }
}