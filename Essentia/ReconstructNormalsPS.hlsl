
#include "FrameCommon.hlsli"

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

cbuffer PerFrame : register(b1)
{
    PerFrameData FrameData;
}

Texture2D<float> DepthTexture : register(t0);

SamplerState AnisoSampler       : register(s0);
SamplerState LinearWrapSampler  : register(s2);
SamplerState PointClampSampler  : register(s3);

float3 WorldPosFromDepth(float2 uv, float depth)
{
    float z = depth * 2.0 - 1.0;

    float4 clipSpacePosition = float4(uv * 2.0 - 1.0, z, 1.0);
    float4 viewSpacePosition = mul(clipSpacePosition, FrameData.CamInvProjection);

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    float4 worldSpacePosition = mul(viewSpacePosition, FrameData.CamInvView);

    return worldSpacePosition.xyz;
}

float3 ReconstructNormal(float2 uv)
{
    float2 stc = uv;
    float depth = DepthTexture.Sample(LinearWrapSampler, stc).x;
    //float3 Pos = VSPositionFromDepth(uv, depth);
    //int bestZV;
    //int bestZH;
    float2 screenSize = FrameData.ScreenSize;
    float4 H;
    H.x = DepthTexture.Sample(LinearWrapSampler, stc - float2(1 / screenSize.x, 0)).x;
    H.y = DepthTexture.Sample(LinearWrapSampler, stc + float2(1 / screenSize.x, 0)).x;
    H.z = DepthTexture.Sample(LinearWrapSampler, stc - float2(2 / screenSize.x, 0)).x;
    H.w = DepthTexture.Sample(LinearWrapSampler, stc + float2(2 / screenSize.x, 0)).x;
    
    float2 he = abs(H.xy * H.zw * rcp(2 * H.zw - H.xy) - depth);
    float3 hDeriv;

    if (he.x > he.y)
    {
        float2 uvz = stc - float2(2 / screenSize.x, 0);
        float2 uvx = stc - float2(1 / screenSize.x, 0);
        float3 posZ = WorldPosFromDepth(uvz, H.z);
        float3 posX = WorldPosFromDepth(uvx, H.x);
        hDeriv = posX - posZ;
    }
    else
    {
        float2 uvw = stc - float2(2 / screenSize.x, 0);
        float2 uvy = stc - float2(1 / screenSize.x, 0);
        float3 posW = WorldPosFromDepth(uvw, H.w);
        float3 posY = WorldPosFromDepth(uvy, H.y);
        hDeriv = posW - posY;
    }
        
    float4 V;
    V.x = DepthTexture.Sample(LinearWrapSampler, stc - float2(0, 1 / screenSize.y)).x;
    V.y = DepthTexture.Sample(LinearWrapSampler, stc + float2(0, 1 / screenSize.y)).x;
    V.z = DepthTexture.Sample(LinearWrapSampler, stc - float2(0, 2 / screenSize.y)).x;
    V.w = DepthTexture.Sample(LinearWrapSampler, stc + float2(0, 2 / screenSize.y)).x;
    
    float2 ve = abs(V.xy * V.zw * rcp(2 * V.zw - V.xy) - depth);
    float3 vDeriv;
    if (ve.x > ve.y)
    {
        float2 uvz = stc - float2(0, 2 / screenSize.y);
        float2 uvx = stc - float2(0, 1 / screenSize.y);
        float3 posZ = WorldPosFromDepth(uvz, V.z);
        float3 posX = WorldPosFromDepth(uvx, V.x);
        vDeriv = posX - posZ;
    }
    else
    {
        float2 uvy = stc - float2(0, 1 / screenSize.y);
        float2 uvw = stc - float2(0, 2 / screenSize.y);
        float3 posY = WorldPosFromDepth(uvy, V.w);
        float3 posW = WorldPosFromDepth(uvw, V.y);
        vDeriv = posW - posY;
    }
    
    return normalize(cross(hDeriv, vDeriv));
}

float4 main(VertexToPixel input) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}