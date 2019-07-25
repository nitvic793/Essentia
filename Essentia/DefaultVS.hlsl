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

	output.Position = mul(float4(input.Position, 1.f), worldViewProjection);
	output.Normal = normalize(mul(input.Normal, (float3x3)World));
	output.UV = input.UV;
	output.Tangent = input.Tangent;

	return output;
}