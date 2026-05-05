#include "Common.hlsli"

float4 PS(PS_INPUT input) : SV_Target
{
    // 1. 벡터 정규화
	float3 N = normalize(input.normal);
	float3 L = normalize(LightDir);
    
    // 2. 램버트 코사인 법칙 (내적 후 0~1 클램핑)
    // 빛이 정면일 때 1, 측면일 때 0이 됩니다.
    // 빛이 표면 뒤쪽에서 오면 내적 값이 음수가 되므로, 이를 0으로 잘라버립니다 (saturate).
	float NdotL = saturate(dot(N, L));

    // 3. 표면의 기본 색상 및 조명 색상
	float3 baseColor = input.col.rgb;
	float3 lightColor = float3(1.0f, 1.0f, 1.0f); // 흰색 조명이라 가정
    
    // 4. 최종 색상 연산 (Base Color * NdotL * Light Color)
	float3 finalColor = baseColor * NdotL * lightColor;
    
	return float4(finalColor, input.col.a);
}