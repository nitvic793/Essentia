#include "VoxelCommon.hlsli"

RWTexture3D<unorm float4> VoxelGrid : register(u0);
Texture2D AlbedoTexture : register(t0);

SamplerState BaseSampler : register(s0);
SamplerState LinearWrapSampler : register(s2);

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
   
    
    float4 result = AlbedoTexture.Sample(LinearWrapSampler, input.UV);
    
    VoxelGrid[writecoord] = result;
}