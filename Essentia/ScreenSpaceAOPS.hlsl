
//Reference: https://github.com/d3dcoder/d3d12book/blob/master/Chapter%2021%20Ambient%20Occlusion/Ssao/Shaders/Ssao.hlsl
// Frank D Luna, Chapter 21 Ambient Occlusion 

struct VertexOut
{
    float4 Position : SV_POSITION;
    float3 PosV     : POSITION;
    float2 UV       : TEXCOORD0;
};

cbuffer AOParams : register(b0)
{
    float4x4    Projection;
    float4x4    InvProjection;
    float4x4    ProjectionTex;
    float4      OffsetVectors[14];
    float4      BlurWeights[3];
    float2      ScreenSize;
    float       OcclusionRadius;
    float       OcclusionFadeStart;
    float       OcclusionFadeEnd;
    float       SurfaceEpsilon;
}

static const int gSampleCount = 14;

SamplerState BasicSampler               : register(s0);
SamplerComparisonState ShadowSampler    : register(s1);
SamplerState LinearWrapSampler          : register(s2);
SamplerState PointClampSampler          : register(s3);

Texture2D<float> DepthTexture      : register(t0);
Texture2D RandomVecTexture  : register(t1);

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

float OcclusionFunction(float distZ)
{
    float occlusion = 0.0f;
    if (distZ > SurfaceEpsilon)
    {
        float fadeLength = OcclusionFadeEnd - OcclusionFadeStart;
        occlusion = saturate((OcclusionFadeEnd - distZ) / fadeLength);
    }
	
    return occlusion;
}

float4 main(VertexOut input) : SV_TARGET
{
    float2 texelSize = 1.f / ScreenSize;
    float depth = DepthTexture.SampleLevel(LinearWrapSampler, input.UV, 0.f).r;
    float3 vsPos = VSPositionFromDepth(input.UV, depth);
    float3 normal = ReconstructNormal(vsPos);
    float3 n = normal;
    
    float pz = depth;
    pz = NdcDepthToViewDepth(pz);
    float3 p = (pz / input.PosV.z) * input.PosV;
  
    float3 randVec = 2.0f * RandomVecTexture.SampleLevel(LinearWrapSampler, 4.0f * input.UV, 0.0f).rgb - 1.0f;
    randVec = normalize(randVec);
    //randVec = lerp(randVec, float3(BlurWeights[0].xyz), 0.5f);
    float occlusionSum = 0.0f;

    for (int i = 0; i < gSampleCount; ++i)
    {
        float3 offset = reflect(OffsetVectors[i].xyz, randVec);
        float flip = sign(dot(offset, n));
        float3 q = p + flip * OcclusionRadius * offset;
		
        float4 projQ = mul(float4(q, 1.0f), ProjectionTex);
        projQ /= projQ.w;

        float rz = DepthTexture.SampleLevel(LinearWrapSampler, projQ.xy, 0.0f).r;
        rz = NdcDepthToViewDepth(rz);

        float3 r = (rz / q.z) * q;
        float distZ = p.z - r.z;
        float dp = max(dot(n, normalize(r - p)), 0.0f);

        float occlusion = dp * OcclusionFunction(distZ);
        occlusionSum += occlusion;
    }
	
    occlusionSum /= gSampleCount;
    float access = 1.0f - occlusionSum;
    float output = saturate(pow(access, 6.0f));
    
    return float4(output.rrr, 1.f);
}