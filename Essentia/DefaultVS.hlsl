#include "Common.hlsli"

cbuffer PerObject : register(b0)
{
	float4x4 World;
	float4x4 View;
	float4x4 Projection;
	float4x4 WorldViewProjection;
	float4x4 PrevWorldViewProjection;
};

cbuffer PerFrame : register(b1)
{
    float4x4 ViewProjectionTex;
}

cbuffer ShadowBuffer : register(b2)
{
    float4x4 ShadowView;
    float4x4 ShadowProjection;
}

PixelInput main(VertexInput input)
{
	PixelInput output;
	
    float4x4 shadowVP = mul(mul(World, ShadowView), ShadowProjection);
    float4 worldPos = mul(float4(input.Position, 1.0f), World);
	
	output.Position = mul(float4(input.Position, 1.f), WorldViewProjection);
	output.Normal = normalize(mul(input.Normal, (float3x3)World));
	output.UV = input.UV;
	output.Tangent = normalize(mul(input.Tangent, (float3x3)World));;
    output.WorldPos = worldPos.xyz;
    output.ShadowPos = mul(float4(input.Position, 1.0f), shadowVP);
    output.SSAOPos = mul(worldPos, ViewProjectionTex);
	return output;
}