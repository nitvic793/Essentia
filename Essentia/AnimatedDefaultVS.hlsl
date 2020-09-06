#include "Common.hlsli"
#include "FrameCommon.hlsli"

#define CMaxBones 128

cbuffer PerObject : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
    float4x4 WorldViewProjection;
    float4x4 PrevWorldViewProjection;
};

cbuffer PerFrame : register(b1)
{
    PerFrameData FrameData;
}

cbuffer PerArmature : register(b2)
{
    matrix Bones[CMaxBones];
};

float4x4 SkinTransform(float4 weights, uint4 boneIndices)
{
	// Calculate the skin transform from up to four bones and weights
    float4x4 skinTransform =
			Bones[boneIndices.x] * weights.x +
			Bones[boneIndices.y] * weights.y +
			Bones[boneIndices.z] * weights.z +
			Bones[boneIndices.w] * weights.w;
    return skinTransform;
}

void SkinVertex(inout float4 position, inout float3 normal, float4x4 skinTransform)
{
    position = mul(position, skinTransform);
    normal = mul(normal, (float3x3) skinTransform);
}

PixelInput main(VertexAnimatedInput input)
{
    PixelInput output;
    float4x4 skinTransform = SkinTransform(input.SkinWeights, input.SkinIndices);
    
    float4 position = float4(input.Position, 1.f);
    if(input.SkinWeights.x !=0)
    {
        SkinVertex(position, input.Normal, skinTransform);
    }
    
    float4x4 shadowVP = mul(mul(World, FrameData.ShadowView), FrameData.ShadowProjection);
    float4 worldPos = mul(position, World);
	
    output.Position = mul(position, WorldViewProjection);
    output.Normal = normalize(mul(input.Normal, (float3x3) World));
    output.UV = input.UV;
    output.Tangent = normalize(mul(input.Tangent, (float3x3) World));;
    output.WorldPos = worldPos.xyz;
    output.ShadowPos = mul(position, shadowVP);
    output.SSAOPos = mul(worldPos, FrameData.ViewProjectionTex);
    return output;
}