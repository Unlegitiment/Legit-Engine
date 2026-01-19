struct VSIn
{
    float2 Pos : POSITION0;
    float2 UV : UVCOORD0;
};
struct VSOut
{
    float4 pos : SV_Position;
    float2 UV : UVCOORD0;
};
VSOut VS(VSIn pos)
{
    VSOut v;
    v.pos = float4(pos.Pos, 0, 1);
    v.UV = pos.UV;
    return v;
}