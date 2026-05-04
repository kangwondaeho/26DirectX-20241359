// 상수 버퍼 및 구조체 정의
cbuffer ConstantBuffer : register(b0)
{
    matrix WorldViewProj;
    matrix World;
    float3 LightDir;
    float padding1;
    float3 ViewPos;
    float padding2;
}

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float4 col : COLOR;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float4 col : COLOR;
};