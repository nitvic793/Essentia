#include "Common.hlsli"

cbuffer LightBuffer : register(b0)
{
	DirectionalLight DirLight;
}

float4 main(PixelInput input) : SV_TARGET
{
	float3 light = CalculateDirectionalLight(input.Normal, DirLight);
	return float4(light, 1.0f);
}