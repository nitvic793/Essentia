struct VertexInput
{
	float3 Position : POSITION;
	float2 UV		: TEXCOORD;
	float3 Normal	: NORMAL;
	float3 Tangent	: TANGENT0;
};

struct PixelInput
{
	float4 Position		: SV_POSITION;
	float4 PrevPosition	: POSITION;
};

cbuffer PerObject : register(b0)
{
	float4x4 World;
	float4x4 View;
	float4x4 Projection;
	float4x4 PrevWorld;
};

PixelInput main(VertexInput input)
{
	PixelInput output;
	matrix worldViewProjection = mul(mul(World, View), Projection);
	matrix prevWorldViewProjection = mul(mul(PrevWorld, View), Projection);

	output.Position = mul(float4(input.Position, 1.f), worldViewProjection);
	output.PrevPosition = mul(float4(input.Position, 1.f), prevWorldViewProjection);
	return output;
}