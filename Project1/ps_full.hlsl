//Texture2D DepthTex : register(t0);


struct PSIn
{
    float4 Pos : SV_Position;
    float2 UV : TEXCOORD0;
};

SamplerState LinearClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
    AddressW = CLAMP;
};


float4 PS(float4 pos : SV_Position, float2 UV : UVCOORD0) : SV_Target
{
    //float4 bloom = float4(0, 0, 0, 0);
    //float span = 20;
	////float span = 6 + 5 * sin(5*g_time.x);
    //int tt = abs(span);
    //for (int i = -tt; i <= tt; ++i)
    //{
    //    float ofs = i;
    //    bloom += DepthTex.Sample(LinearClamp, UV + float2(0, ofs / 100)) / (2 * tt + 1);
    //}
    return float4(0,0,0,0); //DepthTex.Sample(LinearClamp, UV) + bloom;
}