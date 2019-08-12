#include "Common.hlsli"

cbuffer PerObject : register(b0)
{
	float4x4 World;
	float4x4 View;
	float4x4 Projection;
};

struct VertexToPixelSky
{
	float4 Position		: SV_POSITION;
	float3 UVW			: TEXCOORD;
};

VertexToPixelSky main(VertexInput input)
{
	// Set up output
	VertexToPixelSky output;

	// Make a view matrix with NO translation
	matrix viewNoMovement = View;
	viewNoMovement._41 = 0;
	viewNoMovement._42 = 0;
	viewNoMovement._43 = 0;

	// Calculate output position
	matrix viewProj = mul(viewNoMovement, Projection);
	output.Position = mul(float4(input.Position, 1.0f), viewProj);

	// Ensure our polygons are at max depth
	output.Position.z = output.Position.w;

	// Use the cube's vertex position as a direction in space
	// from the origin (center of the cube)
	output.UVW = input.Position;

	return output;
}