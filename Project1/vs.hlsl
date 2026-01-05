struct VSIn
{
    float3 vPosition : POSITION0;
    float2 vColor : UVCOORD0;
};

cbuffer Transform : register(b0) // BUFFER 0 
{
    matrix m_Projection;
    matrix m_View;
    matrix m_Model;
};

struct VertexOut
{
    float4 Pos : SV_Position;
    float2 Col : UVCOORD0;
};

VertexOut LAG(VSIn IO) 
{
    VertexOut output;

    float4 localPos = float4(IO.vPosition, 1.0f);

    float4 worldPos = mul(localPos, m_Model);
    float4 viewPos = mul(worldPos, m_View);
    float4 projPos = mul(viewPos, m_Projection);

    output.Pos = projPos;
    output.Col = IO.vColor;
    return output;
}