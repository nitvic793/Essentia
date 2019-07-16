#include "Common.hlsli"

PixelInput main(VertexInput input)
{
	PixelInput output;
	output.Position = float4(input.Position, 1.f);
	output.Normal = input.Normal;
	output.UV = input.UV;
	output.Tangent = input.Tangent;
	return output;
}