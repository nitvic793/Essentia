

#include "VoxelCommon.hlsli"

cbuffer PerObject : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
    float4x4 WorldViewProjection;
    float4x4 PrevWorldViewProjection;
};

cbuffer ShadowBuffer : register(b1)
{
    float4x4 ShadowView;
    float4x4 ShadowProjection;
}

GSInput main(VertexInput input)
{
    GSInput output;
	
    float4x4 shadowVP = mul(mul(World, ShadowView), ShadowProjection);
    float4 worldPos = mul(float4(input.Position, 1.0f), World);
	
    output.Position = worldPos; // mul(float4(input.Position, 1.f), WorldViewProjection); //
    output.Normal = normalize(mul(input.Normal, (float3x3) World));
    output.UV = input.UV;
    output.Tangent = normalize(mul(input.Tangent, (float3x3) World));;
    output.WorldPos = worldPos.xyz;
    output.ShadowPos = mul(float4(input.Position, 1.0f), shadowVP);
    return output;
}