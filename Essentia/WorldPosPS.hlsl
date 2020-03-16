
#include "Common.hlsli"

float4 main(PixelInput input) : SV_TARGET
{
	return float4(input.WorldPos.xyz, 1.0f);
}