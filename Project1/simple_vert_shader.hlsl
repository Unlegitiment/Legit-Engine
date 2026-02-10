struct VSIn
{
    float3 Position : POSITION0;
    //float2 TextureUV : TEXCOORD0;
};
cbuffer LagMatrices : register(b0)
{
    matrix m_Projection; // 
    matrix m_View; // shifts universe to camera
    matrix m_Model; // looks  
};
struct VSOut
{
    float4 Pos : SV_Position;
};
float4 VS_MainQuad(float3 Position : POSITION0) : SV_Position
{
    float4 pos = float4(Position, 1.0f);
    
    float4 WorldPos = mul(pos, m_Model);
    float4 ViewPos = mul(WorldPos, m_View);
    float4 ProjectionPos = mul(ViewPos, m_Projection);
    return ProjectionPos;
}

float4 PS_MainQuad(float4 Pos : SV_Position) : SV_Target0
{
    return float4(1, 1, 1, 1);
}
