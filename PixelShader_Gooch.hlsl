#include "Common.hlsli"

float4 PS(PS_INPUT input) : SV_Target
{
    float3 N = normalize(input.normal);
    float3 L = normalize(LightDir);
    float3 V = normalize(ViewPos - input.worldPos);
    
    // 1. 빛의 방향에 따른 보간 비율 (intensity)
    float t = (dot(N, L) + 1.0f) * 0.5f;

    // 2. Base Color 및 Warm/Cool 색상 정의
    float3 baseColor = input.col.rgb;
    float3 coolColor = float3(0.0f, 0.0f, 0.55f) + 0.25f * baseColor;
    float3 warmColor = float3(0.3f, 0.3f, 0.0f) + 0.25f * baseColor; // 0.25로 수정

    // 3. 디퓨즈 색상 보간
    float3 diffuseGooch = lerp(coolColor, warmColor, t);

    // 4. 스타일라이즈드 하이라이트 계산 (단순 덧셈이 아닌 비율 s 사용)
    float3 R = reflect(-L, N);
    float s = saturate(100.0f * dot(R, V) - 97.0f); // 0~1 사이로 클램핑 (x^+)
    float3 highlightColor = float3(1.0f, 1.0f, 1.0f);

    // 5. 최종 색상 조합 (Lerp 사용)
    float3 finalColor = lerp(diffuseGooch, highlightColor, s);
    
    return float4(finalColor, input.col.a);
}