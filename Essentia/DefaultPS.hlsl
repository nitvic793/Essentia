#include "Common.hlsli"
#include "Lighting.hlsli"

cbuffer LightBuffer : register(b0)
{
	DirectionalLight	DirLight;
	PointLight			PointLights;
	float3				CameraPosition;
}

sampler		BasicSampler	: register(s0);
Texture2D	AlbedoTexture	: register(t0);
Texture2D	NormalTexture	: register(t1);

float4 main(PixelInput input) : SV_TARGET
{
	float3 normalSample = NormalTexture.Sample(BasicSampler, input.UV).xyz;
	float3 normal = CalculateNormalFromSample(normalSample, input.UV, input.Normal, input.Tangent);

	float3 light = CalculateDirectionalLight(normal, DirLight) + CalculatePointLight(normal, CameraPosition, input.WorldPos, PointLights);
	float4 texColor = AlbedoTexture.Sample(BasicSampler, input.UV);
	float3 color = texColor.rgb;

	float3 output = light * color;
	output = pow(abs(output), 1.f / 2.2f); 

	return float4(output, 1.0f);
}