
#include "Common.hlsli"
#include "Lighting.hlsli"

cbuffer LightBuffer : register(b0)
{
	DirectionalLight DirLights[CMaxDirLights];
	PointLight PointLights[CMaxPointLights];
	SpotLight SpotLights[CMaxSpotLights];
	float3 CameraPosition;
	float Padding1;
	uint DirLightCount;
	uint PointLightCount;
	uint SpotLightCount;
	float Padding2;
}

cbuffer ShadowBuffer : register(b2)
{
	float4x4 ShadowView;
	float4x4 ShadowProjection;
}

SamplerState BasicSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);
SamplerState LinearWrapSampler : register(s2);
SamplerState PointClampSampler : register(s3);

Texture2D ShadowMapDirLight : register(t8);
///Credits: https://www.alexandre-pestana.com/volumetric-lights/

static const float G_SCATTERING = 0.5f;
static const int NB_STEPS = 32;

float ComputeScattering(float lightDotView)
{
	float result = 1.0f - G_SCATTERING * G_SCATTERING;
	result /= (4.0f * PI * pow(1.0f + G_SCATTERING * G_SCATTERING - (2.0f * G_SCATTERING) * lightDotView, 1.5f));
	return result;
}

float ShadowAmount(float4 shadowPos)
{
	float2 shadowUV = shadowPos.xy / shadowPos.w * 0.5f + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y;
	float depthFromLight = shadowPos.z / shadowPos.w;
	float shadowAmount = ShadowMapDirLight.SampleCmpLevelZero(ShadowSampler, shadowUV, depthFromLight);
	return shadowAmount;
}

float4 main(PixelInput input) : SV_TARGET
{
	float3 worldPos = input.WorldPos;
	float4 shadowPos = input.ShadowPos;
	float3 startPos = CameraPosition;
	float3 rayVector = worldPos - startPos;
	float3 rayDirection = normalize(rayVector);
	float rayLength = length(rayVector);
	float3 sunDir = normalize(DirLights[0].Direction);
	float stepLength = rayLength / NB_STEPS;
	float3 step = rayDirection * stepLength;
	
	float3 currPos = startPos;
	float4x4 shadowViewProj = ShadowView * ShadowProjection;
	float2 shadowUV = shadowPos.xy / shadowPos.w * 0.5f + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y;
	float depthFromLight = shadowPos.z / shadowPos.w;
	float3 sunColor = float3(1.f, 1.f, 1.f);
	float3 accumFog = float3(0.f, 0.f, 0.f);
	for (int i = 0; i < NB_STEPS; ++i)
	{
		float4 worldInShadowCameraSpace = mul(float4(currPos, 1.0f), shadowViewProj);
		worldInShadowCameraSpace /= worldInShadowCameraSpace.w;
		float shadowMapValue = ShadowMapDirLight.Load(uint3(shadowUV, 0)).r;
		if (shadowMapValue > depthFromLight)
		{
			accumFog += ComputeScattering(dot(rayDirection, sunDir)).xxx * sunColor;
		}
		currPos += step;
	}
	
	accumFog /= NB_STEPS;
	
	return float4(accumFog, 1.0f);
}