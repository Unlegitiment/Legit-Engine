struct PSInput
{
    float4 Pos : SV_Position;
    float3 Col : COLOR0;
    float2 UV : UVCOORD0;
};;

//Texture2D m_Texture : register(t0); // BIND RESOURCE TO TEXTURE0
//SamplerState MeshTextureSampler : register(s0);

struct PSOut
{
	float4 color : SV_Target; // Is this for RenderTarget specifications?
};

PSOut main(PSInput input)
{
	PSOut output = (PSOut) 0;

    output.color = float4(input.Col, 1.0);
	return output;
}