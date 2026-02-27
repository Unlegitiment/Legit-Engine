struct VSIn
{
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
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
    float3 Normal : NORMAL0;
    float3 FragPos : CockButt4; // WHY DOES IT NEED A SEMANTIC IF IT DOESN'T CARE ABOUT THE NAMEEEE LMAO
};
VSOut VS_MainQuad(VSIn IAIn)
{
    VSOut vsOut;
    float4 pos = float4(IAIn.Position, 1.0f);
    float4 WorldPos = mul(pos, m_Model);
    float4 ViewPos = mul(WorldPos, m_View);
    float4 ProjectionPos = mul(ViewPos, m_Projection);
    vsOut.Pos = ProjectionPos;
    vsOut.Normal = IAIn.Normal;
    vsOut.FragPos = float3(mul(float4(IAIn.Position, 1.0), m_Model).xyz);
    return vsOut;
}
cbuffer LagLighting : register(b0)
{
    float4 m_ObjectColor;
    float4 m_LightColor;
    float4 m_LightPosition;
    float4 m_ViewPos;
};
float4 PS_MainQuad(VSOut vsOut) : SV_Target0
{
    return float4(m_ObjectColor);
}

float4 PS_PhongBasic(VSOut vsOut) : SV_Target0
{
    float ambientStrength = 0.1;
    float3 ambient = ambientStrength * m_LightColor.rgb;
    
    
    float3 norm = normalize(vsOut.Normal);
    float3 lightDir = normalize(m_LightPosition - vsOut.FragPos);
    float diff = max(dot(norm, lightDir), 0);
    float3 diffuse = diff * m_LightColor;
    
    float specularStrength = 0.5;
    
    float3 viewDir = normalize(m_ViewPos.xyz - vsOut.FragPos);
    float3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // 32 is shinyness
    float3 specular = specularStrength * spec * m_LightColor;
    
    
    float3 result = (ambient + diffuse + specular) * m_ObjectColor.rgb;
    
    return float4(result, 1.0);
}