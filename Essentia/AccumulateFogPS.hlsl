
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
Texture2D NoiseTexture		: register(t9);
///Credits: https://www.alexandre-pestana.com/volumetric-lights/

static const float G_SCATTERING = -0.2f;
static const int NB_STEPS = 32;

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

float4 main(VertexToPixel input) : SV_TARGET
{
	float3 startPos = CameraPosition;
    float depth = SceneDepthTexture.Sample(LinearWrapSampler, input.UV).r;
    float3 depthPos = VSPositionFromDepth(input.UV, depth);
    float3 rayVector = depthPos - startPos;
	float3 rayDirection = normalize(rayVector);
	float rayLength = length(rayVector);
	float3 sunDir = normalize(DirLights[0].Direction);
	float stepLength = rayLength / NB_STEPS;
	float3 step = rayDirection * stepLength;
	
	float3 currPos = startPos;
    float4x4 shadowViewProj = mul(ShadowView, ShadowProjection);

	float3 sunColor = float3(1.f, 1.f, 1.f);
	float3 accumFog = float3(0.f, 0.f, 0.f);
	
	[unroll]
	for (int i = 0; i < NB_STEPS; ++i)
	{
		float4 worldInShadowCameraSpace = mul(float4(currPos, 1.0f), shadowViewProj);
		worldInShadowCameraSpace /= worldInShadowCameraSpace.w; 
        worldInShadowCameraSpace.xy = worldInShadowCameraSpace.xy * 0.5f + 0.5f;
        worldInShadowCameraSpace.y = 1 - worldInShadowCameraSpace.y;
        float shadowMapValue = ShadowMapDirLight.Sample(LinearWrapSampler, worldInShadowCameraSpace.xy).r;
        if (shadowMapValue > worldInShadowCameraSpace.z)
        {
            accumFog += ComputeScattering(dot(rayDirection, sunDir)).xxx * sunColor;
        }
		currPos += step;
	}
	
	accumFog /= NB_STEPS;
	
	return float4(accumFog, 1.0f);
}