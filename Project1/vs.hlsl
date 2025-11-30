struct VSIn
{
    float3 vPosition : POSITION;
    float3 Col : COLOR;
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
    float3 Col : COLOR;
};

VertexOut main(VSIn IO) 
{
    VertexOut output;

    float4 localPos = float4(IO.vPosition, 1.0f);

    float4 worldPos = mul(localPos, m_Model);
    float4 viewPos = mul(worldPos, m_View);
    float4 projPos = mul(viewPos, m_Projection);

    output.Pos = projPos;
    output.Col = IO.Col;

    return output;
}