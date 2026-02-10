struct VSIn
{
    float3 Pos : POSITION0;
    float2 UV : UVCOORD0;
};

struct VSOut
{
    float4 Pos : SV_Position;
    float2 UV : UVCOORD0;
};
cbuffer LagMatrices : register(b0)
{
    matrix m_Projection; // 
    matrix m_View; // shifts universe to camera
    matrix m_Model; // looks  
};

VSOut VS_Main(VSIn input)
{
    VSOut output;
    float4 pos = float4(input.Pos, 1.0f);
    
    float4 WorldPos = mul(pos, m_Model);
    float4 ViewPos = mul(WorldPos, m_View);
    float4 ProjectionPos = mul(ViewPos, m_Projection);
    
    
    output.Pos = ProjectionPos;
    output.UV = input.UV;
    return output;
}

struct PSOut
{
    float4 Color : SV_Target0;
};

Texture2D Texture : register(t0);

SamplerState LinearClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
    AddressW = CLAMP;
};
PSOut PS_Main(VSOut In)
{
    const float offset = 1.0 / 300.0;
    float2 offsets[9] =
    {
        
        float2(-offset, offset), // top-left
        float2(0.0f, offset), // top-center
        float2(offset, offset), // top-right
        float2(-offset, 0.0f), // center-left
        float2(0.0f, 0.0f), // center-center
        float2(offset, 0.0f), // center-right
        float2(-offset, -offset), // bottom-left
        float2(0.0f, -offset), // bottom-center
        float2(offset, -offset) // bottom-right    
    };
    float kernel[9] =
    {
        1, 1, 1,
        1, -8, 1,
        1, 1, 1
    };
    
    float3 sampleTex[9];
    for (int i = 0; i < 9; i++)
    {
        sampleTex[i] = float3(Texture.Sample(LinearClamp, In.UV + offsets[i]).xyz);
    }
    
    float3 col = float3(0,0,0);
    for (int j = 0; j < 9; j++)
        col += sampleTex[j] * kernel[j];
    PSOut o;
    o.Color = In.Pos;
    return o;
}