#include "Common.hlsli"
#include "AnimationCommon.hlsli"

cbuffer PerObject : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
    float4x4 WorldViewProjection;
    float4x4 PrevWorldViewProjection;
};

cbuffer PerArmature : register(b1)
{
    matrix Bones[CMaxBones];
};


PixelInput main(VertexAnimatedInput input)
{
    PixelInput output;
	
    float4x4 skinTransform = SkinTransform(input.SkinWeights, input.SkinIndices, Bones);
    
    float4 position = float4(input.Position, 1.f);
    if (input.SkinWeights.x != 0)
    {
        SkinVertex(position, input.Normal, input.Tangent, skinTransform);
    }
    
    float4 worldPos = mul(position, World);
    
    output.Position = mul(position, WorldViewProjection);
    output.Normal = normalize(mul(input.Normal, (float3x3) World));
    output.UV = input.UV;
    output.Tangent = normalize(mul(input.Tangent, (float3x3) World));;
    output.WorldPos = worldPos.xyz;
    output.ShadowPos = float4(0.f, 0.f, 0.f, 0.f);
    output.SSAOPos = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}