#include "DofCommon.hlsli"

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

cbuffer DepthOfFieldParams : register(b0)
{
	float NearZ; // Camera NearZ
	float FarZ; // Camera FarZ
	float FocusPlaneZ; // Focus Z Plane from Camera
	float Scale; // DOF Scale
    float ResolutionWidth;
    float ResolutionHeight;
};


Texture2D			SceneTexture	: register(t0);
Texture2D<float>	DepthTexture	: register(t1);

SamplerState		BasicSampler	: register(s0);
SamplerState		LinearWrapSampler : register(s2);
SamplerState		PointClampSampler : register(s3);

float4 main(VertexToPixel input) : SV_TARGET
{
    float depth = DepthTexture.Sample(PointClampSampler, input.uv).r;
    float centerSize = GetBlurSize(depth, FocusPlaneZ, Scale, NearZ, FarZ);
    float3 sceneColor = SceneTexture.Sample(LinearWrapSampler, input.uv).rgb;
    return float4(sceneColor, centerSize);
}