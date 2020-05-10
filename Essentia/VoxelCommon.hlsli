struct GSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 WorldPos : POSITION0;
    float4 ShadowPos : SHADOW_POS;
};

struct GSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 WorldPos : POSITION0;
    float4 ShadowPos : SHADOW_POS;
};

struct VertexInput
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT0;
};


cbuffer VoxelParams : register(b2)
{
    float3 VoxelRCPSize;
    float Padding;
    float3 VoxelGridMaxPoint;
    float Padding2;
    float3 VoxelGridMinPoint;
    float Padding3;
    float3 VoxelGridCenter;
    float Padding4;
    float3 VoxelGridSize;
    float Padding5;
    float4x4 VoxelGridViewProjMatrices[3];
}