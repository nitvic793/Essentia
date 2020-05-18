#include "VoxelCommon.hlsli"
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

RWTexture3D<unorm float4> VoxelGrid : register(u0);
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
    uint3 dim;
    VoxelGrid.GetDimensions(dim.x, dim.y, dim.z);
    VoxelGrid[uint3(1, 1, 1)] = float4(1.f.xxxx);
    
    float3 diff = (input.WorldPos - VoxelGridCenter) * VoxelRadianceDataResRCP * VoxelRadianceDataSizeRCP;
    float3 uvw = diff * float3(0.5f, -0.5f, 0.5f) + 0.5f;
    uint3 writecoord = floor(uvw * VoxelRadianceDataRes);
    
    float4x4 shadowViewProj = mul(ShadowView, ShadowProjection);
    
    float4 shadowPos = mul(float4(input.WorldPos, 1), shadowViewProj);
    float3 materialColor = AlbedoTexture.Sample(LinearWrapSampler, input.UV).rgb;
    float3 dirLight = CalculateDirectionalLight(normalize(input.Normal), DirLights[0]) * ShadowAmount(shadowPos); // <- Enable once GI is finished
    
    float4 result = float4(materialColor * dirLight, 1.f);
    
    VoxelGrid[writecoord] = result;
}