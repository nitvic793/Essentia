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

	//output.Position = mul(float4(input.Position, 1.f), worldViewProjection);
	output.Position = float4(input.Position, 1.f);
	output.Normal = input.Normal;
	output.UV = input.UV;
	output.Tangent = input.Tangent;

	return output;
}