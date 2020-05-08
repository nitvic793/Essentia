#include "VoxelCommon.hlsli"

static const uint CNumVertices = 3;
static const uint CVoxelGridSize = 128;

cbuffer PerObject : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
    float4x4 WorldViewProjection;
    float4x4 PrevWorldViewProjection;
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
};

[maxvertexcount(CNumVertices)]
void main(
	triangle GSInput input[3],
	inout TriangleStream< GSOutput > output
)
{
    float3 worldSpaceFaceNormal = abs(input[0].Normal + input[1].Normal + input[2].Normal);
    uint maxi = worldSpaceFaceNormal[1] > worldSpaceFaceNormal[0] ? 1 : 0;
    maxi = worldSpaceFaceNormal[2] > worldSpaceFaceNormal[maxi] ? 2 : maxi;
    
    for (uint i = 0; i < CNumVertices; i++)
	{
		GSOutput element;
        element = input[i];
        element.Position = float4((element.Position.xyz - VoxelGridCenter) / VoxelGridSize, 1.f);
        if(maxi == 0)
        {
            element.Position.xyz = element.Position.zyx;
        }
        else if(maxi == 1)
        {
            element.Position.xyz = element.Position.xzy;
        }
        
        element.Position.xy /= CVoxelGridSize;
        element.Position.z = 1.f;
        
		output.Append(element);
	}
    
    output.RestartStrip();
}