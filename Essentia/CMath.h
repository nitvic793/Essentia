#pragma once

#include <cereal/cereal.hpp>
#include <DirectXMath.h>

struct Vector3
{
	float X;
	float Y;
	float Z;

	Vector3() {}

	Vector3(const DirectX::XMFLOAT3& vector)
	{
		X = vector.x;
		Y = vector.y;
		Z = vector.z;
	}

	void operator=(DirectX::XMFLOAT3& vector)
	{
		X = vector.x;
		Y = vector.y;
		Z = vector.z;
	}

	operator DirectX::XMFLOAT3()
	{
		return DirectX::XMFLOAT3(X, Y, Z);
	}

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(
			CEREAL_NVP(X),
			CEREAL_NVP(Y),
			CEREAL_NVP(Z)
		);
	}
};

struct Vector4
{
	float X;
	float Y;
	float Z;
	float W;
};



//***************************************************************************************
// Reference: https://github.com/d3dcoder/d3d12book/blob/master/Common/MathHelper.h
// MathHelper.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Helper math class.
//***************************************************************************************
class MathHelper
{
public:
	static float RandF()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	// Returns random float in [a, b).
	static float RandF(float a, float b)
	{
		return a + RandF() * (b - a);
	}

	static DirectX::XMFLOAT4X4 Identity4x4()
	{
		static DirectX::XMFLOAT4X4 I(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		return I;
	}
};
