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
};

Texture2D DOFTexture : register(t0);
Texture2D SceneTexture : register(t1);

SamplerState BasicSampler : register(s0);
SamplerState LinearWrapSampler : register(s2);
SamplerState PointClampSampler : register(s3);

float4 main(VertexToPixel input) : SV_TARGET
{
    float4 dofColor = DOFTexture.Sample(LinearWrapSampler, input.uv).rgba;
    float blurSize = dofColor.a;
    float3 sceneColor = SceneTexture.Sample(LinearWrapSampler, input.uv).rgb;
    float3 outputColor = 0.f.xxx;
    
    if (blurSize >= 1.f)
        outputColor = dofColor.rgb;
    else
        outputColor = lerp(sceneColor, dofColor.rgb, clamp(blurSize - 0.5f, 0.f, 1.f));
    //outputColor = lerp(sceneColor, dofColor.rgb, 1.f - clamp(1.f / blurSize, 0.f, 1.f));
    return float4(outputColor, 1.0f);
}