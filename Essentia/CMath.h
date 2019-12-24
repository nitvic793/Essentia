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
