
//Reference: https://github.com/d3dcoder/d3d12book/blob/master/Chapter%2021%20Ambient%20Occlusion/Ssao/Shaders/SsaoBlur.hlsl
// Frank D Luna, Chapter 21 Ambient Occlusion 

struct VertexOut
{
    float4 Position : SV_POSITION;
    float3 PosV : POSITION;
    float2 UV : TEXCOORD0;
};

cbuffer AOParams : register(b0)
{
    float4x4 Projection;
    float4x4 InvProjection;
    float4x4 ProjectionTex;
    float4x4 InvView;
    float4x4 InvViewProj;
    float4 OffsetVectors[14];
    float4 BlurWeights[3];
    float2 ScreenSize;
    float OcclusionRadius;
    float OcclusionFadeStart;
    float OcclusionFadeEnd;
    float SurfaceEpsilon;
}

cbuffer BlurParams : register(b1)
{
    float2 TexOffset; // Blur Direction
}

SamplerState BasicSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);
SamplerState LinearWrapSampler : register(s2);
SamplerState PointClampSampler : register(s3);

Texture2D<float> DepthTexture : register(t0);
Texture2D RandomVecTexture : register(t1);
Texture2D<float> InputTexture : register(t2);

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

float3 ReconstructNormal(float3 viewSpacePos)
{
    return normalize(cross(ddx(viewSpacePos), ddy(viewSpacePos)));
}

float NdcDepthToViewDepth(float z_ndc)
{
    // z_ndc = A + B/viewZ, where gProj[2,2]=A and gProj[3,2]=B.
    float viewZ = Projection[3][2] / (z_ndc - Projection[2][2]);
    return viewZ;
}

static const int gBlurRadius = 5;

float4 main(VertexOut input) : SV_TARGET 
{
    float blurWeights[12] =
    {
        BlurWeights[0].x, BlurWeights[0].y, BlurWeights[0].z, BlurWeights[0].w,
        BlurWeights[1].x, BlurWeights[1].y, BlurWeights[1].z, BlurWeights[1].w,
        BlurWeights[2].x, BlurWeights[2].y, BlurWeights[2].z, BlurWeights[2].w,
    };
    
    float2 texelSize = 1.f / ScreenSize;
    float depth = DepthTexture.SampleLevel(LinearWrapSampler, input.UV, 0.f).r;
    float3 vsPos = VSPositionFromDepth(input.UV, depth);
    float3 normal = ReconstructNormal(vsPos);
    float3 n = normal;
    
    float4 color = blurWeights[gBlurRadius] * InputTexture.SampleLevel(PointClampSampler, input.UV, 0.0);
    float totalWeight = blurWeights[gBlurRadius];
    float3 centerNormal = normal;
    float centerDepth = NdcDepthToViewDepth(depth);
    
    for (float i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
		// We already added in the center weight.
        if (i == 0)
            continue;

        float2 tex = input.UV + i * TexOffset;

        float depthValue = DepthTexture.SampleLevel(LinearWrapSampler, tex, 0.0f).r;
        vsPos = VSPositionFromDepth(input.UV, depthValue);
        float3 neighborNormal = ReconstructNormal(VSPositionFromDepth(tex, depthValue));
        float neighborDepth = NdcDepthToViewDepth(depthValue);
	    
        bool normalAngle = (dot(neighborNormal, centerNormal) >= 0.8f);
        bool depthCheck = (abs(neighborDepth - centerDepth) <= 0.2f);
        if (normalAngle && depthCheck)
        {
            float weight = blurWeights[i + gBlurRadius];

			// Add neighbor pixel to blur.
            color += weight * InputTexture.SampleLevel(
                LinearWrapSampler, tex, 0.0);
		
            totalWeight += weight;
        }
    }

	// Compensate for discarded samples by making total weights sum to 1.
    return color / totalWeight;
}