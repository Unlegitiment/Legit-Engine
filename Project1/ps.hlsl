struct PSInput
{
	float4 Pos : SV_Position;
    float3 Col : COLOR;
};
struct PSOut
{
	float4 color : SV_Target; // Is this for RenderTarget specifications?
};

PSOut Main(PSInput input)
{
	PSOut output = (PSOut) 0;
	output.color = float4(input.Col, 1.0); // This is for white.
	
	return output;
}