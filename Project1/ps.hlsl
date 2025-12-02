struct PSInput
{
	float4 Pos : SV_Position;
    float4 Col : COLOR;
};
struct PSOut
{
	float4 color : SV_Target; // Is this for RenderTarget specifications?
};

PSOut Main(PSInput input)
{
	PSOut output = (PSOut) 0;
	output.color = input.Col; // This is for white.
	return output;
}