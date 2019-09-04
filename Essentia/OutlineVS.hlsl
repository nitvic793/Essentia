#include "Common.hlsli"

cbuffer PerObject : register(b0)
{
	float4x4 World;
	float4x4 View;
	float4x4 Projection;
};

PixelInput main(VertexInput input)
{
	PixelInput output;
	matrix worldViewProjection = mul(mul(World, View), Projection);

	output.Normal = normalize(mul(input.Normal, (float3x3)World));
	output.Position = mul(float4(input.Position + input.Normal * 0.02f, 1.f), worldViewProjection);
	output.UV = input.UV;
	output.Tangent = normalize(mul(input.Tangent, (float3x3)World));;
	output.WorldPos = mul(float4(input.Position, 1.0f), World).xyz;

	return output;
}