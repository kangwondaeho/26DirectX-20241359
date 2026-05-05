#include "Common.hlsli"

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;
    
    // 위치 변환 (순서 주의: 벡터 * 행렬)
	output.pos = mul(float4(input.pos, 1.0f), WorldViewProj);
	output.worldPos = mul(float4(input.pos, 1.0f), World).xyz;
    
    // 법선 변환 (픽셀 셰이더로 넘겨주기만 합니다)
	output.normal = mul(input.normal, (float3x3) World);
    
	output.col = input.col;

	return output;
}