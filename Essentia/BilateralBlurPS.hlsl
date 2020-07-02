
struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

cbuffer BilaterBlurParams : register(b0)
{
	uint2 FullResolution;
	uint2 LowResolution;
}

Texture2D<float4> DepthTexture : register(t0);
Texture2D<float4> LightAccumTexture : register(t1);
SamplerState BasicSampler : register(s0);
SamplerState LinearWrapSampler : register(s2);

float4 main(VertexToPixel input) : SV_TARGET
{
	uint2 fullScreenCoords = FullResolution * input.uv;
	uint2 lowResCoords = LowResolution * input.uv;
	float upSampledDepth = DepthTexture.Load(int3(fullScreenCoords, 0)).x;

	float3 color = 0.0f.xxx;
	float totalWeight = 0.0f;
	int xOffset = fullScreenCoords.x % 2 == 0 ? -1 : 1;
	int yOffset = fullScreenCoords.y % 2 == 0 ? -1 : 1;

	int2 offsets[] =
	{
		int2(0, 0),
		int2(0, yOffset),
		int2(xOffset, 0),
		int2(xOffset, yOffset)
	};

	[unroll]
	for (int i = 0; i < 4; i++)
	{

		float3 downscaledColor = LightAccumTexture.Load(int3(lowResCoords + offsets[i], 0)).xyz;

		float downscaledDepth = DepthTexture.Load(int3(lowResCoords + offsets[i], 0)).r;

		float currentWeight = 1.0f;
		currentWeight *= max(0.0f, 1.0f - (0.05f) * abs(downscaledDepth - upSampledDepth));

		color += downscaledColor * currentWeight;
		totalWeight += currentWeight;

	}

	float3 volumetricLight;
	const float epsilon = 0.0001f;
	volumetricLight.xyz = color / (totalWeight + epsilon);
	return float4(volumetricLight.xyz, 1.0f);
}