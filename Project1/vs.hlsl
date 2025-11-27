struct VSIn
{
    float3 vPosition : POSITION;
    float3 Col : COLOR;
};

struct VertexOut
{
    float4 Pos : SV_Position;
    float3 Col : COLOR;
};

VertexOut main(VSIn IO) 
{
    VertexOut output = (VertexOut) 0;
    output.Pos = float4(IO.vPosition, 1.00);
    output.Col = IO.Col;
    return output;
}