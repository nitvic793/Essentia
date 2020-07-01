#include "FrameCommon.hlsli"


cbuffer FrameDataBuffer : register(b0)
{
    PerFrameData FrameData;
}

//Texture3D<float4> InputVoxel : register(t0);
RWStructuredBuffer<VoxelType> InputVoxel : register(u0);
RWTexture3D<float4> OutputVoxel : register(u1);

static const uint CBlockSize = 4;

[numthreads(CBlockSize, CBlockSize, CBlockSize)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint id = Flatten3D(DTid, FrameData.VoxelData.VoxelRadianceDataRes);
    const VoxelType input = InputVoxel[id];
    
    const float4 color = DecodeColor(input.ColorMask);
    const float3 normal = DecodeNormal(input.NormalMask);
    
    int hasColor = (color.a > 0.f);
    //float3 currentColor = OutputVoxel[DTid].rgb;
    //float3 resultColor = lerp(currentColor, input.rgb, 0.2f) * hasColor;
    //float4 currentColor = OutputVoxel.Load(int4(DTid.xyz, 0));
    float3 result = color.rgb;//lerp(currentColor.rgb, color.rgb, 0.2f);
    
    OutputVoxel[DTid] = float4(result, 1.f) * hasColor;
    InputVoxel[id].ColorMask = 0;
}
