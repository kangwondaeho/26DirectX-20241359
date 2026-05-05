#include "Common.hlsli"

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;
    
    // 수정됨: 벡터(float4)를 먼저 쓰고, 행렬(Matrix)을 뒤에 곱합니다!
	output.pos = mul(float4(input.pos, 1.0f), WorldViewProj);
	output.worldPos = mul(float4(input.pos, 1.0f), World).xyz;
    
    // 수정됨: 법선(Normal) 변환도 마찬가지로 순서를 뒤집습니다.
	float3 N = normalize(mul(input.normal, (float3x3) World));
    
	float3 L = normalize(LightDir);
    
    // 조명 연산은 그대로!
	float NdotL = saturate(dot(N, L));
	float3 baseColor = input.col.rgb;
	float3 lightColor = float3(1.0f, 1.0f, 1.0f);
    
	output.col.rgb = baseColor * NdotL * lightColor;
	output.col.a = input.col.a;
    
	output.normal = N;

	return output;
}