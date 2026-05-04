#include "Common.hlsli"

float4 PS(PS_INPUT input) : SV_Target
{
    float3 N = normalize(input.normal);
    float3 L = normalize(LightDir);
    float3 V = normalize(ViewPos - input.worldPos);
    
    float NdotL = dot(N, L);
    float intensity = (NdotL + 1.0f) * 0.5f;

    float3 baseColor = input.col.rgb;
    float3 coolColor = float3(0.0f, 0.0f, 0.55f) + 0.25f * baseColor;
    float3 warmColor = float3(0.3f, 0.3f, 0.0f) + 0.5f * baseColor;

    float3 diffuseGooch = lerp(coolColor, warmColor, intensity);

    float3 R = reflect(-L, N);
    float spec = pow(max(dot(R, V), 0.0f), 32.0f);
    float3 specular = float3(1.0f, 1.0f, 1.0f) * spec;

    float3 finalColor = diffuseGooch + specular;
    
    return float4(finalColor, input.col.a);
}