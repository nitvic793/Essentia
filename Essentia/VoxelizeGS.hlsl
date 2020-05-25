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

//Reference: https://github.com/turanszkij/WickedEngine/blob/master/WickedEngine/objectPS_voxelizer.hlsl

[maxvertexcount(CNumVertices)]
void main(
	triangle GSInput input[3],
	inout TriangleStream< GSOutput > outputStream
)
{
    GSOutput output[CNumVertices];
    
    float3 facenormal = abs(input[0].Normal + input[1].Normal + input[2].Normal);
    uint maxi = facenormal[1] > facenormal[0] ? 1 : 0;
    maxi = facenormal[2] > facenormal[maxi] ? 2 : maxi;
    
    [unroll]
    for (uint i = 0; i < CNumVertices; ++i)
    {
		// World space -> Voxel grid space:
        output[i].Position.xyz = (input[i].Position.xyz - VoxelGridCenter) * VoxelRadianceDataSizeRCP;

		// Project onto dominant axis:
		
        output[i].Position.xyz = (maxi == 0) ? output[i].Position.zyx : (maxi == 1) ? output[i].Position.xzy : output[i].Position.xyz;
        //(maxi == 0) ? (output[i].Position.xyz = output[i].Position.zyx) : 0.f;
        //(maxi == 1) ? (output[i].Position.xyz = output[i].Position.xzy) : 0.f;
    } 
    
    // Expand triangle to get fake Conservative Rasterization:
    //float2 side0N = normalize(output[1].Position.xy - output[0].Position.xy);
    //float2 side1N = normalize(output[2].Position.xy - output[1].Position.xy);
    //float2 side2N = normalize(output[0].Position.xy - output[2].Position.xy);
    //output[0].Position.xy += normalize(side2N - side0N);
    //output[1].Position.xy += normalize(side0N - side1N);
    //output[2].Position.xy += normalize(side1N - side2N);
    
    [unroll]
    for (uint j = 0; j < CNumVertices; j++)
    {
		// Voxel grid space -> Clip space
        output[j].Position.xy *= VoxelRadianceDataResRCP;
        output[j].Position.zw = 1;

		// Append the rest of the parameters as is:
        output[j].UV = input[j].UV;
        output[j].Normal = input[j].Normal;
        output[j].WorldPos = input[j].Position.xyz;
        output[j].ShadowPos = input[j].ShadowPos;
        outputStream.Append(output[j]);
    }

    outputStream.RestartStrip();
   
}