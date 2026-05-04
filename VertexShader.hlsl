#include "Common.hlsli"

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    
    output.pos = mul(float4(input.pos, 1.0f), WorldViewProj);
    output.worldPos = mul(float4(input.pos, 1.0f), World).xyz;
    output.normal = normalize(mul(float4(input.normal, 0.0f), World).xyz);
    output.col = input.col;
    
    return output;
}