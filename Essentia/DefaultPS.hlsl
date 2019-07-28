#include "Common.hlsli"

cbuffer LightBuffer : register(b0)
{
	DirectionalLight DirLight;
}

sampler		BasicSampler	: register(s0);
Texture2D	AlbedoTexture	: register(t0);
Texture2D	NormalTexture	: register(t1);

float4 main(PixelInput input) : SV_TARGET
{
	float3 normalSample = NormalTexture.Sample(BasicSampler, input.UV).xyz;
	float3 normal = CalculateNormalFromSample(normalSample, input.UV, input.Normal, input.Tangent);
	float3 light = CalculateDirectionalLight(normal, DirLight);
	float3 color = AlbedoTexture.Sample(BasicSampler, input.UV).rgb;
	return float4(light * color, 1.0f);
}