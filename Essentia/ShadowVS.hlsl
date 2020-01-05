#include "Common.hlsli"

cbuffer ShadowBuffer : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
};

float4 main(VertexInput input) : SV_POSITION
{
    matrix worldViewProj = mul(mul(World, View), Projection);
    return mul(float4(input.Position, 1.0f), worldViewProj);
}