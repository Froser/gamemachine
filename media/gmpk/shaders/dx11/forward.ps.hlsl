Texture2D diffuseTex;
SamplerState samLinear;

struct PS_INPUT
{
    float3 normal      : NORMAL;
    float2 texcoord    : TEXCOORD0;
};

float4 PS( PS_INPUT input ) : SV_Target
{
    return diffuseTex.Sample( samLinear, input.texcoord);
}
