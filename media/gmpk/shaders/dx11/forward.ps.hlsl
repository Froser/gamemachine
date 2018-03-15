
struct PS_INPUT
{
    float3 normal      : NORMAL;
    float2 texcoord    : TEXCOORD0;
};

float4 PS( PS_INPUT input ) : SV_Target
{
    return float4( 1.0f, 1.0f, 0.0f, 1.0f );    // Yellow, with Alpha = 1
}
