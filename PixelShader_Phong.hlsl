#include "Common.hlsli"

float4 PS(PS_INPUT input) : SV_Target
{
    // 핵심 1. 보간된 법선을 다시 정규화(Normalize) 합니다.
    // 정점 사이를 선형 보간하면서 길이가 미세하게 변한 법선을 다시 길이 1로 맞춥니다.
	float3 N = normalize(input.normal);
	float3 L = normalize(LightDir);
	float3 V = normalize(ViewPos - input.worldPos);
    
    // 핵심 2. 주변광 (Ambient) - 그림자 영역이 완전 까매지지 않게 기본 밝기 부여
	float3 ambient = 0.1f * input.col.rgb;

    // 핵심 3. 난반사 (Diffuse / Lambertian)
	float NdotL = saturate(dot(N, L));
	float3 diffuse = input.col.rgb * NdotL;

    // 핵심 4. 정반사 (Specular / 하이라이트)
	float3 R = reflect(-L, N);
    // pow() 함수를 통해 빛이 맺히는 범위를 좁고 강하게 만듭니다. (32.0f는 광택도/Shininess)
	float spec = pow(max(dot(R, V), 0.0f), 32.0f);
	float3 specular = float3(1.0f, 1.0f, 1.0f) * spec;

    // 5. 최종 색상 = 주변광 + 난반사 + 정반사
	float3 finalColor = ambient + diffuse + specular;
    
	return float4(finalColor, input.col.a);
}