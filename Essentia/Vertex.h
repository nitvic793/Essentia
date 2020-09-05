#pragma once

#include <DirectXMath.h>
#include "OGLMath.h"

constexpr int CMaxBonesPerVertex = 4;

struct VertexBoneData
{
	uint32_t	IDs[CMaxBonesPerVertex];
	float		Weights[CMaxBonesPerVertex];

	void AddBoneData(uint32_t boneID, float weight)
	{
		for (uint32_t i = 0; i < CMaxBonesPerVertex; i++) {
			if (Weights[i] == 0.0) {
				IDs[i] = boneID;
				Weights[i] = weight;
				return;
			}
		}
	}
};

struct Vertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT3 Tangent;
	float		padding[5];
};