#include "Constants.hlsli"

float4x4 SkinTransform(float4 weights, uint4 boneIndices, matrix bones[CMaxBones])
{
	// Calculate the skin transform from up to four bones and weights
    float4x4 skinTransform =
			bones[boneIndices.x] * weights.x +
			bones[boneIndices.y] * weights.y +
			bones[boneIndices.z] * weights.z +
			bones[boneIndices.w] * weights.w;
    return skinTransform;
}

void SkinVertex(inout float4 position, inout float3 normal, float4x4 skinTransform)
{
    position = mul(position, skinTransform);
    normal = mul(normal, (float3x3) skinTransform);
}