
//Reference: https://github.com/d3dcoder/d3d12book/blob/master/Chapter%2021%20Ambient%20Occlusion/Ssao/Shaders/Ssao.hlsl
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

static const int gSampleCount = 14;

SamplerState BasicSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);
SamplerState LinearWrapSampler : register(s2);
SamplerState PointClampSampler : register(s3);

Texture2D<float> DepthTexture : register(t0);
Texture2D RandomVecTexture : register(t1);

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

float3 WorldPosFromDepth(float2 uv, float depth)
{
    float z = depth * 2.0 - 1.0;

    float4 clipSpacePosition = float4(uv * 2.0 - 1.0, z, 1.0);
    float4 viewSpacePosition = mul(clipSpacePosition, InvProjection);

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    float4 worldSpacePosition = mul(viewSpacePosition, InvView);

    return worldSpacePosition.xyz;
}


float3 ReconstructNormal(float2 uv)
{
    float2 stc = uv;
    float depth = DepthTexture.Sample(LinearWrapSampler, stc).x;
    //float3 Pos = VSPositionFromDepth(uv, depth);
    //int bestZV;
    //int bestZH;
    float4 H;
    H.x = DepthTexture.Sample(LinearWrapSampler, stc - float2(1 / ScreenSize.x, 0)).x;
    H.y = DepthTexture.Sample(LinearWrapSampler, stc + float2(1 / ScreenSize.x, 0)).x;
    H.z = DepthTexture.Sample(LinearWrapSampler, stc - float2(2 / ScreenSize.x, 0)).x;
    H.w = DepthTexture.Sample(LinearWrapSampler, stc + float2(2 / ScreenSize.x, 0)).x;
    
    //float3 HPos[4];
    //HPos[0] = VSPositionFromDepth(stc - float2(1 / ScreenSize.x, 0), H.x);
    //HPos[1] = VSPositionFromDepth(stc + float2(1 / ScreenSize.x, 0), H.y);
    //HPos[2] = VSPositionFromDepth(stc - float2(2 / ScreenSize.x, 0), H.z);
    //HPos[3] = VSPositionFromDepth(stc + float2(2 / ScreenSize.x, 0), H.w);
    float2 he = abs(H.xy * H.zw * rcp(2 * H.zw - H.xy) - depth);
    float3 hDeriv;

    if (he.x > he.y)
    {
        float2 uvz = stc - float2(2 / ScreenSize.x, 0);
        float2 uvx = stc - float2(1 / ScreenSize.x, 0);
        float3 posZ = WorldPosFromDepth(uvz, H.z);
        float3 posX = WorldPosFromDepth(uvx, H.x);
        hDeriv = posX - posZ;
    }
    else
    {
        float2 uvw = stc - float2(2 / ScreenSize.x, 0);
        float2 uvy = stc - float2(1 / ScreenSize.x, 0);
        float3 posW = WorldPosFromDepth(uvw, H.w);
        float3 posY = WorldPosFromDepth(uvy, H.y);
        hDeriv = posW - posY;
    }
        
    float4 V;
    V.x = DepthTexture.Sample(LinearWrapSampler, stc - float2(0, 1 / ScreenSize.y)).x;
    V.y = DepthTexture.Sample(LinearWrapSampler, stc + float2(0, 1 / ScreenSize.y)).x;
    V.z = DepthTexture.Sample(LinearWrapSampler, stc - float2(0, 2 / ScreenSize.y)).x;
    V.w = DepthTexture.Sample(LinearWrapSampler, stc + float2(0, 2 / ScreenSize.y)).x;
    
    //float3 VPos[4];
    //VPos[0] = VSPositionFromDepth(stc - float2(0, 1 / ScreenSize.y), V.x);
    //VPos[1] = VSPositionFromDepth(stc + float2(0, 1 / ScreenSize.y), V.y);
    //VPos[2] = VSPositionFromDepth(stc - float2(0, 2 / ScreenSize.y), V.z);
    //VPos[3] = VSPositionFromDepth(stc + float2(0, 2 / ScreenSize.y), V.w);
    
    float2 ve = abs(V.xy * V.zw * rcp(2 * V.zw - V.xy) - depth);
    float3 vDeriv;
    if (ve.x > ve.y)
    {
        float2 uvz = stc - float2(0, 2 / ScreenSize.y);
        float2 uvx = stc - float2(0, 1 / ScreenSize.y);
        float3 posZ = WorldPosFromDepth(uvz, V.z);
        float3 posX = WorldPosFromDepth(uvx, V.x);
        vDeriv = posX - posZ;
    }
    else
    {
        float2 uvy = stc - float2(0, 1 / ScreenSize.y);
        float2 uvw = stc - float2(0, 2 / ScreenSize.y);
        float3 posY = WorldPosFromDepth(uvy, V.w);
        float3 posW = WorldPosFromDepth(uvw, V.y);
        vDeriv = posW - posY;
    }
    
    return normalize(cross(hDeriv, vDeriv));
}

float3 ReconstructPosition(in float2 uv, in float z, in float4x4 InvVP)
{
    float x = uv.x * 2.0f - 1.0f;
    float y = (1.0 - uv.y) * 2.0f - 1.0f;
    float4 position_s = float4(x, y, z, 1.0f);
    float4 position_v = mul(InvVP, position_s);
    return position_v.xyz / position_v.w;
}

float3 ReconstructNormalV2(float2 uv)
{
    float2 uv0 = uv; // center
    float2 uv1 = uv + float2(1, 0) / ScreenSize; // right 
    float2 uv2 = uv + float2(0, 1) / ScreenSize; // top

    float depth0 = DepthTexture.SampleLevel(PointClampSampler, uv0, 0).r;
    float depth1 = DepthTexture.SampleLevel(PointClampSampler, uv1, 0).r;
    float depth2 = DepthTexture.SampleLevel(PointClampSampler, uv2, 0).r;

    float3 P0 = ReconstructPosition(uv0, depth0, InvProjection);
    float3 P1 = ReconstructPosition(uv1, depth1, InvProjection);
    float3 P2 = ReconstructPosition(uv2, depth2, InvProjection);

    float3 normal = normalize(cross(P2 - P0, P1 - P0));
    return normal;
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

inline float rand(inout float seed, in float2 uv)
{
    float result = frac(sin(seed * dot(uv, float2(12.9898, 78.233))) * 43758.5453);
    seed += 1;
    return result;
}

float4 main(VertexOut input) : SV_TARGET
{
    float2 texelSize = 1.f / ScreenSize;
    float depth = DepthTexture.SampleLevel(LinearWrapSampler, input.UV, 0.f).r;
    float3 vsPos = VSPositionFromDepth(input.UV, depth);
    float3 normal = ReconstructNormal(vsPos);
    //float3 normal = ReconstructNormal(input.UV);
    float2 uv = input.UV;
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