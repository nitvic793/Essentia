#include "VoxelCommon.hlsli"

RWTexture3D<unorm float4> VoxelGrid : register(u0);

float3 ScaleAndBias(float3 p)
{
    return 0.5f * p + float3(0.5f.xxx);
}

void main(GSOutput input) //: SV_TARGET
{
    uint3 dim;
    VoxelGrid.GetDimensions(dim.x, dim.y, dim.z);
    float3 voxel = ScaleAndBias(input.WorldPos);
    VoxelGrid[uint3(1, 1, 1)] = float4(1.f.xxxx);
    float3 voxelPos = trunc(input.WorldPos);
    float4 result = float4(1.f, 0.f.xx, 1.f);
    //InterlockedMax(VoxelGrid[uint3(voxelPos)].x, uint(result.x));
    //InterlockedMax(VoxelGrid[uint3(voxelPos)].y, uint(result.y));
    //InterlockedMax(VoxelGrid[uint3(voxelPos)].z, uint(result.z));
    //InterlockedMax(VoxelGrid[uint3(voxelPos)].w, uint(result.w));
    
    VoxelGrid[uint3(voxelPos)] = result;
    //return float4(1.f.xxxx);
}