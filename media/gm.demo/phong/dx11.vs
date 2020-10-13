VS_OUTPUT VS( VS_INPUT input )
{
	VS_OUTPUT output;
	output.Position = float4(input.Position.x, input.Position.y, input.Position.z, 1);

	output.Position = mul(output.Position, GM_WorldMatrix);
	output.WorldPos = output.Position;
	
	output.Position = mul(output.Position, GM_ViewMatrix);
	output.Position = mul(output.Position, GM_ProjectionMatrix);

	output.Normal = normalize(mul(input.Normal, GM_ToFloat3x3(GM_WorldMatrix)));
	return output;
}