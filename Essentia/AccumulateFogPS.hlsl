
#include "Common.hlsli"
#include "Lighting.hlsli"

struct VertexToPixel
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

cbuffer LightBuffer : register(b0)
{
	DirectionalLight DirLights[CMaxDirLights];
	PointLight PointLights[CMaxPointLights];
	SpotLight SpotLights[CMaxSpotLights];
	float3 CameraPosition;
	float NearZ;
	uint DirLightCount;
	uint PointLightCount;
	uint SpotLightCount;
	float FarZ;
}

cbuffer LightAccumBuffer : register(b1)
{
    float4x4 InvProjection;
    float4x4 World;
	uint2	 ScreenResolution;
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
Texture2D SceneDepthTexture	: register(t9);
Texture2D NoiseTexture		: register(t10);
Texture2D WorldPosMap		: register(t11);
///Credits: https://www.alexandre-pestana.com/volumetric-lights/

static const float G_SCATTERING = -0.2f;
static const int NB_STEPS = 16;

float3 VSPositionFromDepth(float2 vTexCoord, float depth)
{
    // Get the depth value for this pixel
    float z = depth;
    // Get x/w and y/w from the viewport position
    float x = vTexCoord.x * 2 - 1;
    float y = (1 - vTexCoord.y) * 2 - 1;
    float4 vProjectedPos = float4(x, y, z, 1.0f);
    // Transform by the inverse projection matrix
    float4 vPositionVS = mul(vProjectedPos, InvProjection);
    // Divide by w to get the view-space position
    return vPositionVS.xyz / vPositionVS.w;
}

float2 ProjectionConstants(float nearZ, float farZ)
{
    float2 projectionConstants;
    projectionConstants.x = farZ / (farZ - nearZ);
    projectionConstants.y = (-farZ * nearZ) / (farZ - nearZ);
    return projectionConstants;
}

float LinearZ(float depth)
{
    float2 projectionConstants = ProjectionConstants(NearZ, FarZ);
    float linearZ = projectionConstants.y / (depth - projectionConstants.x);
    return linearZ;
}

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

static const float ditherPattern[4][4] =
{
	{ 0.0f, 0.5f, 0.125f, 0.625f },
	{ 0.75f, 0.22f, 0.875f, 0.375f },
	{ 0.1875f, 0.6875f, 0.0625f, 0.5625 },
	{ 0.9375f, 0.4375f, 0.8125f, 0.3125 }
};

float4 main(VertexToPixel input) : SV_TARGET
{
	float3 startPos = CameraPosition;
    float3 endPos = WorldPosMap.Sample(LinearWrapSampler, input.UV).xyz;
	float3 rayVector = endPos - startPos;
	float3 rayDirection = normalize(rayVector);
	float rayLength = length(rayVector);
	float3 sunDir = normalize(DirLights[0].Direction);
	float stepLength = rayLength / NB_STEPS;
	float3 step = rayDirection * stepLength;
	
	uint2 screenSpacePosition = input.UV * ScreenResolution;
	float ditherValue = ditherPattern[screenSpacePosition.x % 4][screenSpacePosition.y % 4];
	startPos += step * ditherValue;
	float3 currPos = startPos;
    float4x4 shadowViewProj = mul(ShadowView, ShadowProjection);

    float3 sunColor = DirLights[0].Color;
    float intensity = DirLights[0].Intensity;
	float3 accumFog = float3(0.f, 0.f, 0.f);
	
	[unroll]
	for (int i = 0; i < NB_STEPS; ++i)
	{
        float4 worldInShadowCameraSpace = mul(float4(currPos, 1.0f), shadowViewProj);
        float shadowMapValue = ShadowAmount(worldInShadowCameraSpace);
		//if(shadowMapValue == 1.f)
        accumFog += ComputeScattering(dot(rayDirection, sunDir)).xxx * sunColor * shadowMapValue * intensity;
		currPos += step ;
	}
	
	accumFog /= NB_STEPS;
	
	return float4(accumFog, 1.0f);
}