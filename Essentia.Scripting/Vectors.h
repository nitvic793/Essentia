#pragma once

#include <DirectXMath.h>

namespace es::bindings
{
	using namespace DirectX;

	void Normalize(XMFLOAT3& inVector)
	{
		auto vector = XMLoadFloat3(&inVector);
		XMVector3Normalize(vector);
		XMStoreFloat3(&inVector, vector);
	}

	float Dot(XMFLOAT3& lhs, XMFLOAT3& rhs)
	{
		auto l = XMLoadFloat3(&lhs);
		auto r = XMLoadFloat3(&rhs);
	}
}