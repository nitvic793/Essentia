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

//Reference: https://github.com/KolyaNaichuk/RenderSDK/blob/master/Shaders/VoxelizeGS.hlsl

int FindViewDirectionWithLargestProjectedArea(float3 worldSpaceFaceNormal)
{
    float3x3 viewDirectionMatrix =
    {
        1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
    };
    float3 dotProducts = abs(mul(viewDirectionMatrix, worldSpaceFaceNormal));
    float maxDotProduct = max(max(dotProducts.x, dotProducts.y), dotProducts.z);
    int viewIndex = (maxDotProduct == dotProducts.x) ? 0 : 1;
    viewIndex = (maxDotProduct == dotProducts.y) ? 1 : 2;
    return viewIndex;
}

[maxvertexcount(CNumVertices)]
void main(
	triangle GSInput input[3],
	inout TriangleStream< GSOutput > output
)
{
    float3 worldSpaceFaceNormal = normalize(input[0].Normal + input[1].Normal + input[2].Normal);
    int viewIndex = FindViewDirectionWithLargestProjectedArea(worldSpaceFaceNormal);
    
    float4x4 viewProjMatrix = VoxelGridViewProjMatrices[viewIndex];
    
    [unroll]
    for (uint i = 0; i < CNumVertices; i++)
	{
		GSOutput element;
        element.Normal = input[i].Normal;
        element.Position = mul(float4(input[i].WorldPos, 1.f), viewProjMatrix);
        element.WorldPos = input[i].WorldPos;
        element.UV = input[i].UV;
        element.ShadowPos = input[i].ShadowPos;
        element.Tangent = input[i].Tangent;
        
		output.Append(element);
	}
    
    output.RestartStrip();
}