//Texture2D DepthTex : register(t0);


struct PSIn
{
    float4 Pos : SV_Position;
    float2 UV : UVCOORD0;
};
cbuffer Constants : register(b0)
{
    float2 iResolution;
    float iTime;
};



#define DefaultColor float2(0.0, 0.0, 0.0, 1.0)
#define Radius float(1)
#define Fade float(0.005)
#define AdjustedCenter float2(-0.5, -0.5)

float3 DrawCircle(float2 uv, float radius, float fade)
{
    // Determine the length of the point away from the "center" of the circle.
    float uvLength = length(uv);
    // Apply color to any coordinate that lies within the circle.
    // Black if coord is outside and white if it is inside the circle.
    // The smoothstep function applies the fade to the circle.
    // The min value is the radius, and max value is the radius minus
    // the "fade" variable.
    // This means the fade is applied within the bounds of the circle.
    float step = smoothstep(radius, radius - fade, uvLength);
    float3 circle = float3(step, step, step);
    
    return float3(circle);
}
//mat3 getRotZMat(float a){return mat3(cos(a),-sin(a),0.,sin(a),cos(a),0.,0.,0.,1.);}

//float4 PS(PSIn input) : SV_Target0
//{
//    float4 fragColor = float4(0, 0, 0, 1.0);
//    float2 uv = ((input.Pos.xy / iResolution) * 2.0) - 1.0;
//    float aspect = iResolution.x / iResolution.y;
//    uv.x *= aspect;
//    
//    fragColor.r = DrawCircle(uv, Radius, Fade);
//    
//    return fragColor;
//}

/* This animation is the material of my first youtube tutorial about creative 
   coding, which is a video in which I try to introduce programmers to GLSL 
   and to the wonderful world of shaders, while also trying to share my recent 
   passion for this community.
                                       Video URL: https://youtu.be/f4s1h2YETNY
*/

float3 palette(float t)
{
    float3 a = float3(0.5, 0.5, 0.5);
    float3 b = float3(0.5, 0.5, 0.5);
    float3 c = float3(1.0, 1.0, 1.0);
    float3 d = float3(0.263, 0.416, 0.557);

    return a + b * cos(6.28318 * (c * t + d));
}

float4 _PS(PSIn input) : SV_Target0
{
    float2 fragCoord = input.Pos.xy;
    float2 uv = (fragCoord * 2.0 - iResolution.xy) / iResolution.y;
    float2 uv0 = uv;
    float3 finalColor = float3(0,0,0);
    
    for (float i = 0.0; i < 4.0; i++)
    {
        uv = frac(uv * 1.5) - 0.5;

        float d = length(uv) * exp(-length(uv0));

        float3 col = palette(length(uv0) + i * .4 + iTime * .4);

        d = sin(d * 8. + iTime) / 8.;
        d = abs(d);

        d = pow(0.01 / d, 1.2);

        finalColor += col * d;
    }
        
    return float4(finalColor, 1.0);
}

float3x3 getRotYMat(float a)
{
    return float3x3(cos(a), 0., sin(a), 0., 1., 0., -sin(a), 0., cos(a));
}
//mat3 getRotZMat(float a){return mat3(cos(a),-sin(a),0.,sin(a),cos(a),0.,0.,0.,1.);}
float4 PS(PSIn input) : SV_Target
{
    // Reconstruct fragCoord from UV
    float2 fragCoord = input.UV * iResolution.xy;
    float2 s = iResolution.xy;

    float t = iTime * 0.2f;
    float c = 0.0f;
    float d = 0.0f;
    float m = 0.0f;

    float3 p = float3((2.0f * fragCoord - s) / s.x, 1.0f);
    float3 r = p - p; // will be overwritten, matches GLSL r = p - p;
    r = p - p; // keep same semantics
    float3 q = p - p;

    // Actually match GLSL exactly:
    r = p - p; // r = p - p
    q = r; // q = r

    // Rotate around Y
    p = mul(p, getRotYMat(-t));

    // q.zx += 10. + vec2(sin(t), cos(t)) * 3.;
    q.z += 10.0f + sin(t) * 3.0f;
    q.x += 10.0f + cos(t) * 3.0f;

    // Outer loop
    [loop]
    for (float i = 1.0f; i > 0.0f; i -= 0.01f)
    {
        c = 0.0f;
        d = 0.0f;
        m = 1.0f;

        // Inner loop
        [unroll]
        for (int j = 0; j < 3; j++)
        {
            // r = max(r *= r *= r *= r = mod(q*m+1.,2.)-1., r.yzx)
            float3 tmp = fmod(q * m + 1.0f, 2.0f) - 1.0f;
            r = tmp;
            r *= r;
            r *= r;
            r *= r;

            float3 ryzx = float3(r.y, r.z, r.x);
            r = max(r, ryzx);

            d = max(d, (0.29f - length(r) * 0.6f) / m) * 0.8f;
            m *= 1.1f;
        }

        q += p * d;
        c = i;

        if (d < 1e-5f)
            break;
    }

    float k = dot(r, r + 0.15f);
    float3 color = float3(1.0f, k, k / c) - 0.8f;

    return float4(color, 1.0f);
}
