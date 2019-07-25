#include "Common.hlsli"

cbuffer LightBuffer : register(b0)
{
	DirectionalLight DirLight;
}

sampler		BasicSampler	: register(s0);
Texture2D	Texture			: register(t0);

float4 main(PixelInput input) : SV_TARGET
{
	float3 light = CalculateDirectionalLight(normalize(input.Normal), DirLight);
	float3 color = Texture.Sample(BasicSampler, input.UV).rgb;
	return float4(light * color, 1.0f);
}