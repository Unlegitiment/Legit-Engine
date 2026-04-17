struct VSInput
{
    float3 Position : POSITION0;
};
//basic
struct VSInput3D
{
    float3 Position : POSITION0;
};
struct VSOut
{
    float4 Pos : SV_Position;
};
cbuffer LagMatrices : register(b0)
{
    matrix m_Projection; // 
    matrix m_View; // shifts universe to camera
    matrix m_Model; // looks  
};
VSOut BasicTranslate(VSInput vs)
{
    VSOut output;
    output.Pos = float4(vs.Position,1);
    return output;
}
VSOut BasicTranslate3D(VSInput3D ia)
{
    VSOut output;
    float4 pos = float4(ia.Position, 1.0);

    float4 WorldPos = mul(pos, m_Model);
    float4 ViewPos = mul(WorldPos, m_View);
    float4 ClipPos = mul(ViewPos, m_Projection); // ← this line was missing

    output.Pos = ClipPos;
    return output;
}

VSOut vs_main(VSInput Input)
{
    return BasicTranslate(Input);
}
VSOut vs_main3d(VSInput3D Input)
{
    return BasicTranslate3D(Input);
}
float4 ps_main(VSOut vs) : SV_Target0
{
    return float4(0, 0.4, 0, 1); // fuck.
}