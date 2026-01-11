struct PSInput
{
	float4 Pos : SV_Position;
    float2 Col : UVCOORD0;
};
Texture2D m_Texture : register(t0); // BIND RESOURCE TO TEXTURE0
SamplerState MeshTextureSampler : register(s0);
struct PSOut
{
	float4 color : SV_Target; // Is this for RenderTarget specifications?
};
cbuffer MyBuffer : register(b0)
{
    float fOpacity;
    float3 padding;
}

PSOut LAG(PSInput input)
{
	PSOut output = (PSOut) 0;
    float2 uv = input.Col;

    float4 c = m_Texture.Sample(MeshTextureSampler, uv);
    c.a *= fOpacity;
    output.color = c;
	return output;
}