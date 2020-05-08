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

[maxvertexcount(CNumVertices)]
void main(
	triangle GSInput input[3],
	inout TriangleStream< GSOutput > output
)
{
    static const float3 CVoxelGridCenter = float3((CVoxelGridSize / 2.f).xxx);
    static const uint2 CVoxelGridRes = uint2(CVoxelGridSize, CVoxelGridSize);
    float3 worldSpaceFaceNormal = abs(input[0].Normal + input[1].Normal + input[2].Normal);
    uint maxi = worldSpaceFaceNormal[1] > worldSpaceFaceNormal[0] ? 1 : 0;
    maxi = worldSpaceFaceNormal[2] > worldSpaceFaceNormal[maxi] ? 2 : maxi;
    
    for (uint i = 0; i < CNumVertices; i++)
	{
		GSOutput element;
        element = input[i];
        element.Position = float4((element.WorldPos.xyz - CVoxelGridCenter) / CVoxelGridSize, 1.f);
        if(maxi == 0)
        {
            element.Position.xyz = element.WorldPos.zyx;
        }
        else if(maxi == 1)
        {
            element.Position.xyz = element.WorldPos.xzy;
        }
        
        element.Position.xy /= CVoxelGridRes;
        element.Position.z = 1.f;
        
		output.Append(element);
	}
    
    output.RestartStrip();
}