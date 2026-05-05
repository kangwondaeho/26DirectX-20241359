#include "Common.hlsli"

float4 PS(PS_INPUT input) : SV_Target
{
    // 조명 계산이 완전히 사라졌습니다!
    // 레스터라이저가 각 정점(Vertex)에서 계산된 색상을 면적 비율에 따라 
    // 부드럽게 섞어(Interpolation) input.col로 넘겨주기 때문입니다.
    
    return input.col;
}