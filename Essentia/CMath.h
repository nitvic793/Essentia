#pragma once

#include <cereal/cereal.hpp>
#include <DirectXMath.h>
#include <assimp/scene.h>   

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

	static DirectX::XMFLOAT4X4 aiMatrixToXMFloat4x4(const aiMatrix4x4* aiMe)
	{
		auto offset = *aiMe;
		DirectX::XMFLOAT4X4 output;
		//auto mat = XMMatrixTranspose(XMMATRIX(&aiMe->a1));
		DirectX::XMMATRIX mat = DirectX::XMMatrixTranspose(
			DirectX::XMMATRIX(offset.a1, offset.a2, offset.a3, offset.a4,
				offset.b1, offset.b2, offset.b3, offset.b4,
				offset.c1, offset.c2, offset.c3, offset.c4,
				offset.d1, offset.d2, offset.d3, offset.d4));
		DirectX::XMStoreFloat4x4(&output, mat);
		return output;
	}

	static DirectX::XMFLOAT3X3 aiMatrixToXMFloat3x3(const aiMatrix3x3* aiMe)
	{
		DirectX::XMFLOAT3X3 output;
		output._11 = aiMe->a1;
		output._12 = aiMe->a2;
		output._13 = aiMe->a3;

		output._21 = aiMe->b1;
		output._22 = aiMe->b2;
		output._23 = aiMe->b3;

		output._31 = aiMe->c1;
		output._32 = aiMe->c2;
		output._33 = aiMe->c3;

		return output;
	}
};

