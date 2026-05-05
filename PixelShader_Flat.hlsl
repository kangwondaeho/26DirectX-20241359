#include "Common.hlsli"

float4 PS(PS_INPUT input) : SV_Target
{
    // 핵심: 부드럽게 섞여 들어온 법선(input.normal)을 무시하고,
    // 현재 픽셀 주변의 월드 좌표 변화량을 계산해 '진짜 평면의 법선'을 즉석에서 만들어냅니다!
    float3 dpdx = ddx(input.worldPos);
    float3 dpdy = ddy(input.worldPos);
    
    // 두 벡터를 외적(Cross)하면 현재 삼각형 면에 수직인 완벽한 법선이 나옵니다.
    float3 flatNormal = normalize(cross(dpdx, dpdy));
    
    // 이제 이 '평면 법선'을 사용해 빛을 계산합니다. (램버트 모델 적용)
    float3 L = normalize(LightDir);
    float NdotL = saturate(dot(flatNormal, L));

    float3 baseColor = input.col.rgb;
    float3 ambient = 0.1f * baseColor;
    float3 finalColor = ambient + (baseColor * NdotL);
    
    return float4(finalColor, input.col.a);
}