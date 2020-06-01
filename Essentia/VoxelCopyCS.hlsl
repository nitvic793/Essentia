#include "FrameCommon.hlsli"

// flattened array index to 3D array index
inline uint3 unflatten3D(uint idx, uint3 dim)
{
    const uint z = idx / (dim.x * dim.y);
    idx -= (z * dim.x * dim.y);
    const uint y = idx / dim.x;
    const uint x = idx % dim.x;
    return uint3(x, y, z);
}

//Texture3D<float4> InputVoxel : register(t0);
RWTexture3D<float4> OutputVoxel : register(u0);

static const uint CBlockSize = 4;

[numthreads(CBlockSize, CBlockSize, CBlockSize)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    //const float4 input = InputVoxel[DTid];
    //const float4 color = DecodeColor(input.x);
    //const float3 normal = DecodeNormal(input.y);
    
    //float hasColor = float(input.a > 0);
    //float3 currentColor = OutputVoxel[DTid].rgb;
    //float3 resultColor = lerp(currentColor, input.rgb, 0.2f) * hasColor;
    //OutputVoxel[DTid] = float4(input.rgb, 1.f);
}
