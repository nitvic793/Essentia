#include "VoxelCommon.hlsli"
#include "FrameCommon.hlsli"
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

cbuffer ShadowBuffer : register(b1)
{
    float4x4 ShadowView;
    float4x4 ShadowProjection;
}

RWStructuredBuffer<VoxelType> VoxelGrid : register(u0);
//RWTexture3D<unorm float4> VoxelGrid : register(u0);
Texture2D AlbedoTexture : register(t0);

Texture2D ShadowMapDirLight : register(t8);

SamplerState BaseSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);
SamplerState LinearWrapSampler : register(s2);


float ShadowAmount(float4 shadowPos)
{
    float2 shadowUV = shadowPos.xy / shadowPos.w * 0.5f + 0.5f;
    shadowUV.y = 1.0f - shadowUV.y;
    float depthFromLight = shadowPos.z / shadowPos.w;
    float shadowAmount = ShadowMapDirLight.SampleCmpLevelZero(ShadowSampler, shadowUV, depthFromLight);
    return shadowAmount;
}

float3 ScaleAndBias(float3 p)
{
    return 0.5f * p + float3(0.5f.xxx);
}

void main(GSOutput input) //: SV_TARGET
{
   
    float3 diff = (input.WorldPos - VoxelGridCenter) * VoxelRadianceDataResRCP * VoxelRadianceDataSizeRCP;
    float3 uvw = diff * float3(0.5f, -0.5f, 0.5f) + 0.5f;
    uint3 writecoord = floor(uvw * VoxelRadianceDataRes);
    
    float3 normal = normalize(input.Normal);
    float4x4 shadowViewProj = mul(ShadowView, ShadowProjection);
    
    float4 shadowPos = mul(float4(input.WorldPos, 1), shadowViewProj);
    float3 materialColor = AlbedoTexture.Sample(LinearWrapSampler, input.UV).rgb;
    DirectionalLight light = DirLights[0];
    light.Intensity = light.Intensity * light.Intensity * light.Intensity;
    float3 dirLight = CalculateDirectionalLight(normal, light) * materialColor * ShadowAmount(shadowPos); // <- Enable once GI is finished
    
    float3 pointLight = 0.f;
    //for (uint i = 0; i < PointLightCount; ++i)
    //{
    //    pointLight += CalculatePointLight(normal, CameraPosition, input.WorldPos, PointLights[i]);
    //}
    
    float4 result = float4(dirLight + pointLight, 1.f);
    uint id = Flatten3D(writecoord, VoxelRadianceDataRes);
    uint colorEncoded = EncodeColor(result);
    uint normalEncoded = EncodeNormal(normal);
    
    InterlockedMax(VoxelGrid[id].ColorMask, colorEncoded);
    InterlockedMax(VoxelGrid[id].NormalMask, normalEncoded);
    //VoxelGrid[writecoord] = result;
}