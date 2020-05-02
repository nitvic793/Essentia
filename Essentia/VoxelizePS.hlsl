#include "VoxelCommon.hlsli"

RWTexture3D<float4> VoxelGrid : register(u0);

float3 ScaleAndBias(float3 p)
{
    return 0.5f * p + float3(0.5f.xxx);
}

void main(GSOutput input) //: SV_TARGET
{
    uint3 dim;
    VoxelGrid.GetDimensions(dim.x, dim.y, dim.z);
    float3 voxel = ScaleAndBias(input.WorldPos);
    VoxelGrid[uint3(dim * 0.5f)] = float4(1.f, 0.f.xx, 1.f);
    //return float4(1.f.xxxx);
}