#include "DofCommon.hlsli"

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

cbuffer DepthOfFieldParams : register(b0)
{
	float NearZ; // Camera NearZ
	float FarZ; // Camera FarZ
	float FocusPlaneZ; // Focus Z Plane from Camera
	float Scale; // DOF Scale
};


Texture2D			SceneTexture	: register(t0);
Texture2D<float>	DepthTexture	: register(t1);

SamplerState		BasicSampler	: register(s0);
SamplerState		LinearWrapSampler : register(s2);
SamplerState		PointClampSampler : register(s3);

//Reference: http://tuxedolabs.blogspot.com/2018/05/bokeh-depth-of-field-in-single-pass.html


float3 DepthOfField(float2 uv, float focusPoint, float focusScale, out float avgSampleSize)
{
    float depth = DepthTexture.Sample(PointClampSampler, uv).r;
    float centerDepth = (1 - depth) * FarZ;
    float centerSize = GetBlurSize(depth, focusPoint, focusScale, NearZ, FarZ);
    float3 color = SceneTexture.Sample(LinearWrapSampler, uv).rgb;
    float tot = 1.0;
    float totalSampleSize = 0.f;
    float radius = RAD_SCALE;
    float2 pixelSize;
    SceneTexture.GetDimensions(pixelSize.x, pixelSize.y);
    pixelSize = 1.f / pixelSize;
    for (float ang = 0.0; radius < MAX_BLUR_SIZE; ang += GOLDEN_ANGLE)
    {
        float2 tc = uv + float2(cos(ang), sin(ang)) * pixelSize * radius;
        float4 sample = SceneTexture.Sample(LinearWrapSampler, tc).rgba;
        
        float3 sampleColor = sample.rgb;
        float sampleDepth = (1 - DepthTexture.Sample(PointClampSampler, tc).r) * FarZ;
        float sampleSize = sample.a;
        if (sampleDepth > centerDepth)
            sampleSize = clamp(sampleSize, 0.0, centerSize * 2.0);
        float m = smoothstep(radius - 0.5, radius + 0.5, sampleSize);
        color += lerp(color / tot, sampleColor, m);
        tot += 1.0;
        radius += RAD_SCALE / radius;
        totalSampleSize += sampleSize;
    }
    
    avgSampleSize = totalSampleSize / tot;
    return color /= tot;
}

float4 main(VertexToPixel input) : SV_TARGET
{
    float avgSampleSize = 0.f;
    float3 dofColor = DepthOfField(input.uv, FocusPlaneZ, Scale, avgSampleSize);
    return float4(dofColor, avgSampleSize);
}